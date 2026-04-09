#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "httrack-client.h"

enum {
  HTTRACK_CLIENT_WORKERS = 3,
  HTTRACK_CLIENT_CANCEL_THREADS = 2,
  HTTRACK_CLIENT_CANCEL_TRIES = 128,
  HTTRACK_CLIENT_CONNECT_TRIES = 200,
  HTTRACK_CLIENT_RETRY_USEC = 10000,
  HTTRACK_CLIENT_BUFFER_SIZE = 4096,
  HTTRACK_CLIENT_SOCKET_TIMEOUT_SEC = 2,
  HTTRACK_CLIENT_CANCEL_DELAY_USEC = 1000,
  HTTRACK_CLIENT_SETTLE_USEC = 250000,
  HTTRACK_ASSET_PORT = 50081,
  HTTRACK_ASSET_LINKS = 64,
};

static const char *const k_client_host = "127.0.0.1";
static const char *const k_client_project_root = "/tmp/benchamel-httrack";
static const char *const k_client_target_url = "http://127.0.0.1:50081/";
static const char *const k_client_debug_dir = "/tmp/benchamel-httrack-debug";

static pthread_mutex_t state_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t state_cv = PTHREAD_COND_INITIALIZER;
static int client_failed = 0;
static int finished_workers = 0;
static int client_port = 0;
static int asset_server_ready = 0;
static int start_request_ready = 0;
static char start_request_sid[128];

static void mark_failed(const char *what) {
  pthread_mutex_lock(&state_mu);
  if (!client_failed) {
    perror(what);
    client_failed = 1;
  }
  pthread_cond_broadcast(&state_cv);
  pthread_mutex_unlock(&state_mu);
}

static int debug_enabled(void) {
  const char *value = getenv("BENCHAMEL_HTTRACK_DEBUG");
  return value != NULL && value[0] != '\0' && strcmp(value, "0") != 0;
}

static void debug_log(const char *fmt, ...) {
  va_list args;

  if (!debug_enabled()) {
    return;
  }

  fprintf(stderr, "[benchamel][httrack] ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
}

static void debug_write_file(const char *name, const char *contents) {
  char path[512];
  FILE *fp;

  if (!debug_enabled()) {
    return;
  }

  (void)mkdir(k_client_debug_dir, 0777);
  snprintf(path, sizeof(path), "%s/%s", k_client_debug_dir, name);
  fp = fopen(path, "wb");
  if (fp == NULL) {
    return;
  }

  if (contents != NULL) {
    (void)fwrite(contents, 1, strlen(contents), fp);
  }
  fclose(fp);
}

static int is_client_failed(void) {
  int failed;

  pthread_mutex_lock(&state_mu);
  failed = client_failed;
  pthread_mutex_unlock(&state_mu);
  return failed;
}

static void finish_worker(int failed) {
  int should_abort;
  int should_exit;

  should_abort = 0;
  should_exit = 0;

  pthread_mutex_lock(&state_mu);
  if (failed) {
    client_failed = 1;
  }
  finished_workers += 1;
  if (finished_workers == HTTRACK_CLIENT_WORKERS) {
    should_abort = client_failed;
    should_exit = !client_failed;
  }
  pthread_mutex_unlock(&state_mu);

  if (should_abort) {
    abort();
  }
  if (should_exit) {
    exit(0);
  }
}

static void *xrealloc(void *ptr, size_t size) {
  void *new_ptr;

  new_ptr = realloc(ptr, size);
  if (new_ptr == NULL) {
    perror("realloc");
    abort();
  }
  return new_ptr;
}

static char *xstrdup(const char *str) {
  size_t len;
  char *copy;

  len = strlen(str);
  copy = (char *)malloc(len + 1);
  if (copy == NULL) {
    perror("malloc");
    abort();
  }
  memcpy(copy, str, len + 1);
  return copy;
}

static void append_bytes(char **buf, size_t *len, size_t *cap, const char *data,
                         size_t data_len) {
  if (*len + data_len + 1 > *cap) {
    size_t new_cap;

    new_cap = (*cap == 0) ? 256 : *cap;
    while (*len + data_len + 1 > new_cap) {
      new_cap *= 2;
    }
    *buf = (char *)xrealloc(*buf, new_cap);
    *cap = new_cap;
  }

  memcpy(*buf + *len, data, data_len);
  *len += data_len;
  (*buf)[*len] = '\0';
}

static void append_cstr(char **buf, size_t *len, size_t *cap, const char *text) {
  append_bytes(buf, len, cap, text, strlen(text));
}

static void append_urlencoded(char **buf, size_t *len, size_t *cap,
                              const char *text) {
  static const char k_hex[] = "0123456789ABCDEF";
  const unsigned char *ptr;

  for (ptr = (const unsigned char *)text; *ptr != '\0'; ++ptr) {
    char escaped[3];

    if (isalnum(*ptr) || *ptr == '-' || *ptr == '_' || *ptr == '.' ||
        *ptr == '~') {
      append_bytes(buf, len, cap, (const char *)ptr, 1);
    } else if (*ptr == ' ') {
      append_cstr(buf, len, cap, "+");
    } else {
      escaped[0] = k_hex[(*ptr >> 4) & 0x0F];
      escaped[1] = k_hex[*ptr & 0x0F];
      append_cstr(buf, len, cap, "%");
      append_bytes(buf, len, cap, escaped, 2);
    }
  }
}

static void append_form_pair(char **body, size_t *len, size_t *cap,
                             const char *name, const char *value) {
  if (*len > 0) {
    append_cstr(body, len, cap, "&");
  }
  append_urlencoded(body, len, cap, name);
  append_cstr(body, len, cap, "=");
  append_urlencoded(body, len, cap, value);
}

static int connect_once(void) {
  int fd;
  struct sockaddr_in addr;
  struct timeval timeout;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  timeout.tv_sec = HTTRACK_CLIENT_SOCKET_TIMEOUT_SEC;
  timeout.tv_usec = 0;
  (void)setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((unsigned short)client_port);
  if (inet_pton(AF_INET, k_client_host, &addr.sin_addr) != 1) {
    close(fd);
    errno = EINVAL;
    return -1;
  }

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

static int connect_with_retry(int tries) {
  int fd;
  int i;

  for (i = 0; i < tries; ++i) {
    fd = connect_once();
    if (fd >= 0) {
      return fd;
    }
    usleep(HTTRACK_CLIENT_RETRY_USEC);
  }

  return -1;
}

static int send_all(int fd, const char *buf, size_t len) {
  while (len > 0) {
    ssize_t written = send(fd, buf, len, 0);
    if (written < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    buf += (size_t)written;
    len -= (size_t)written;
  }
  return 0;
}

static int recv_all(int fd, char **response) {
  char buf[HTTRACK_CLIENT_BUFFER_SIZE];
  char *data;
  size_t len;
  size_t cap;

  data = NULL;
  len = 0;
  cap = 0;

  while (1) {
    ssize_t read_bytes = recv(fd, buf, sizeof(buf), 0);
    if (read_bytes > 0) {
      append_bytes(&data, &len, &cap, buf, (size_t)read_bytes);
      continue;
    }
    if (read_bytes == 0) {
      break;
    }
    if (errno == EINTR) {
      continue;
    }
    free(data);
    return -1;
  }

  if (data == NULL) {
    data = xstrdup("");
  }
  *response = data;
  return 0;
}

static int http_request(const char *method, const char *path, const char *body,
                        char **response_body) {
  int fd;
  char *request;
  char *response;
  char *header_end;
  size_t request_len;
  size_t body_len;
  size_t request_cap;
  size_t request_used;
  char content_length[64];

  fd = connect_with_retry(HTTRACK_CLIENT_CONNECT_TRIES);
  if (fd < 0) {
    return -1;
  }

  request = NULL;
  request_cap = 0;
  request_used = 0;
  body_len = (body != NULL) ? strlen(body) : 0;

  append_cstr(&request, &request_used, &request_cap, method);
  append_cstr(&request, &request_used, &request_cap, " ");
  append_cstr(&request, &request_used, &request_cap, path);
  append_cstr(&request, &request_used, &request_cap,
              " HTTP/1.0\r\nHost: 127.0.0.1\r\nConnection: close\r\n");
  if (body != NULL) {
    snprintf(content_length, sizeof(content_length), "%zu", body_len);
    append_cstr(&request, &request_used, &request_cap,
                "Content-Type: application/x-www-form-urlencoded\r\n");
    append_cstr(&request, &request_used, &request_cap, "Content-Length: ");
    append_cstr(&request, &request_used, &request_cap, content_length);
    append_cstr(&request, &request_used, &request_cap, "\r\n");
  }
  append_cstr(&request, &request_used, &request_cap, "\r\n");
  if (body != NULL) {
    append_cstr(&request, &request_used, &request_cap, body);
  }

  request_len = strlen(request);
  if (send_all(fd, request, request_len) < 0) {
    free(request);
    close(fd);
    return -1;
  }
  free(request);

  shutdown(fd, SHUT_WR);
  if (recv_all(fd, &response) < 0) {
    close(fd);
    return -1;
  }
  close(fd);

  header_end = strstr(response, "\r\n\r\n");
  if (header_end != NULL) {
    *response_body = xstrdup(header_end + 4);
  } else {
    *response_body = response;
    return 0;
  }

  free(response);
  return 0;
}

static char *copy_trimmed(const char *begin, size_t len) {
  while (len > 0 && isspace((unsigned char)*begin)) {
    begin++;
    len--;
  }
  while (len > 0 && isspace((unsigned char)begin[len - 1])) {
    len--;
  }

  {
    char *copy = (char *)malloc(len + 1);
    if (copy == NULL) {
      perror("malloc");
      abort();
    }
    memcpy(copy, begin, len);
    copy[len] = '\0';
    return copy;
  }
}

static char *extract_input_value(const char *html, const char *name) {
  char pattern[128];
  const char *match;
  const char *value_pos;
  const char *value_end;

  snprintf(pattern, sizeof(pattern), "name=\"%s\"", name);
  match = strstr(html, pattern);
  if (match == NULL) {
    return NULL;
  }

  value_pos = strstr(match, "value=\"");
  if (value_pos == NULL) {
    return NULL;
  }
  value_pos += strlen("value=\"");
  value_end = strchr(value_pos, '"');
  if (value_end == NULL) {
    return NULL;
  }

  return copy_trimmed(value_pos, (size_t)(value_end - value_pos));
}

static char *extract_textarea_value(const char *html, const char *name) {
  char pattern[128];
  const char *match;
  const char *value_pos;
  const char *value_end;

  snprintf(pattern, sizeof(pattern), "name=\"%s\"", name);
  match = strstr(html, pattern);
  if (match == NULL) {
    return NULL;
  }

  value_pos = strchr(match, '>');
  if (value_pos == NULL) {
    return NULL;
  }
  value_pos += 1;
  value_end = strstr(value_pos, "</textarea>");
  if (value_end == NULL) {
    return NULL;
  }

  return copy_trimmed(value_pos, (size_t)(value_end - value_pos));
}

static void wait_for_asset_server(void) {
  pthread_mutex_lock(&state_mu);
  while (!asset_server_ready && !client_failed) {
    pthread_cond_wait(&state_cv, &state_mu);
  }
  pthread_mutex_unlock(&state_mu);
}

static void notify_start_request(const char *sid) {
  pthread_mutex_lock(&state_mu);
  strncpy(start_request_sid, sid, sizeof(start_request_sid) - 1);
  start_request_sid[sizeof(start_request_sid) - 1] = '\0';
  start_request_ready = 1;
  pthread_cond_broadcast(&state_cv);
  pthread_mutex_unlock(&state_mu);
}

static void wait_for_start_request(char *sid, size_t sid_size) {
  pthread_mutex_lock(&state_mu);
  while (!start_request_ready && !client_failed) {
    pthread_cond_wait(&state_cv, &state_mu);
  }
  if (start_request_ready) {
    strncpy(sid, start_request_sid, sid_size - 1);
    sid[sid_size - 1] = '\0';
  } else {
    sid[0] = '\0';
  }
  pthread_mutex_unlock(&state_mu);
}

static char *build_step2_body(const char *sid, const char *projname) {
  char *body;
  size_t len;
  size_t cap;

  body = NULL;
  len = 0;
  cap = 0;
  append_form_pair(&body, &len, &cap, "sid", sid);
  append_form_pair(&body, &len, &cap, "redirect", "");
  append_form_pair(&body, &len, &cap, "todo", "1");
  append_form_pair(&body, &len, &cap, "projname", projname);
  append_form_pair(&body, &len, &cap, "projcateg", "");
  append_form_pair(&body, &len, &cap, "path", k_client_project_root);
  return body;
}

static char *build_step3_body(const char *sid, const char *projname) {
  char *body;
  size_t len;
  size_t cap;

  body = NULL;
  len = 0;
  cap = 0;
  append_form_pair(&body, &len, &cap, "sid", sid);
  append_form_pair(&body, &len, &cap, "redirect", "");
  append_form_pair(&body, &len, &cap, "todo", "1");
  append_form_pair(&body, &len, &cap, "projname", projname);
  append_form_pair(&body, &len, &cap, "projcateg", "");
  append_form_pair(&body, &len, &cap, "path", k_client_project_root);
  append_form_pair(&body, &len, &cap, "urls", k_client_target_url);
  append_form_pair(&body, &len, &cap, "filelist", "");
  return body;
}

static char *build_start_body(const char *sid, const char *projname,
                              const char *command, const char *winprofile,
                              const char *userprofile) {
  char projpath[512];
  char *body;
  size_t len;
  size_t cap;

  snprintf(projpath, sizeof(projpath), "%s/%s/", k_client_project_root,
           projname);

  body = NULL;
  len = 0;
  cap = 0;
  append_form_pair(&body, &len, &cap, "sid", sid);
  append_form_pair(&body, &len, &cap, "redirect", "");
  append_form_pair(&body, &len, &cap, "projpath", projpath);
  append_form_pair(&body, &len, &cap, "command_arg_profile", "path,lang");
  append_form_pair(&body, &len, &cap, "command_do", "start");
  append_form_pair(&body, &len, &cap, "command", command);
  append_form_pair(&body, &len, &cap, "winprofile", winprofile);
  append_form_pair(&body, &len, &cap, "userprofile", userprofile);
  append_form_pair(&body, &len, &cap, "projname", projname);
  append_form_pair(&body, &len, &cap, "projcateg", "");
  append_form_pair(&body, &len, &cap, "path", k_client_project_root);
  append_form_pair(&body, &len, &cap, "urls", k_client_target_url);
  append_form_pair(&body, &len, &cap, "filelist", "");
  append_form_pair(&body, &len, &cap, "todo", "1");
  return body;
}

static char *build_cancel_body(const char *sid) {
  char *body;
  size_t len;
  size_t cap;

  body = NULL;
  len = 0;
  cap = 0;
  append_form_pair(&body, &len, &cap, "sid", sid);
  append_form_pair(&body, &len, &cap, "command", "cancel");
  return body;
}

static char *build_cancel_file_body(const char *sid, const char *command) {
  char *body;
  size_t len;
  size_t cap;

  body = NULL;
  len = 0;
  cap = 0;
  append_form_pair(&body, &len, &cap, "sid", sid);
  append_form_pair(&body, &len, &cap, "command", command);
  return body;
}

static char *extract_cancel_file_command(const char *html) {
  const char *match;
  const char *value_pos;
  const char *value_end;

  match = strstr(html, "form.command.value='cancel-file=");
  if (match == NULL) {
    return NULL;
  }

  value_pos = strchr(match, '\'');
  if (value_pos == NULL) {
    return NULL;
  }
  value_pos += 1;

  value_end = strchr(value_pos, '\'');
  if (value_end == NULL || value_end <= value_pos) {
    return NULL;
  }

  return copy_trimmed(value_pos, (size_t)(value_end - value_pos));
}

static char *build_asset_page(const char *path) {
  char *body;
  size_t len;
  size_t cap;
  int i;

  body = NULL;
  len = 0;
  cap = 0;

  append_cstr(&body, &len, &cap, "<html><body>\n");
  append_cstr(&body, &len, &cap, "<h1>Benchamel HTTrack Fixture</h1>\n");
  append_cstr(&body, &len, &cap, "<p>");
  append_cstr(&body, &len, &cap, path);
  append_cstr(&body, &len, &cap, "</p>\n");

  for (i = 0; i < HTTRACK_ASSET_LINKS; ++i) {
    char link[64];

    snprintf(link, sizeof(link), "<a href=\"/p%d.html\">page%d</a><br/>\n", i,
             i);
    append_cstr(&body, &len, &cap, link);
  }

  append_cstr(&body, &len, &cap, "</body></html>\n");
  return body;
}

static void handle_asset_client(int fd) {
  char request[1024];
  ssize_t nread;
  char method[16];
  char path[256];
  char *body;
  char header[256];

  nread = recv(fd, request, sizeof(request) - 1, 0);
  if (nread <= 0) {
    return;
  }
  request[nread] = '\0';

  method[0] = '\0';
  path[0] = '\0';
  (void)sscanf(request, "%15s %255s", method, path);
  if (path[0] == '\0') {
    strcpy(path, "/");
  }

  usleep(20000);

  if (strcmp(path, "/robots.txt") == 0) {
    body = xstrdup("User-agent: *\nAllow: /\n");
  } else {
    body = build_asset_page(path);
  }

  snprintf(header, sizeof(header),
           "HTTP/1.0 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n"
           "\r\n",
           strlen(body));
  (void)send_all(fd, header, strlen(header));
  (void)send_all(fd, body, strlen(body));
  free(body);
}

static void *asset_server_thread(void *arg) {
  int listen_fd;
  int opt;
  struct sockaddr_in addr;

  (void)arg;

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    mark_failed("socket");
    return NULL;
  }

  opt = 1;
  (void)setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((unsigned short)HTTRACK_ASSET_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(listen_fd);
    mark_failed("bind");
    return NULL;
  }
  if (listen(listen_fd, 16) < 0) {
    close(listen_fd);
    mark_failed("listen");
    return NULL;
  }

  pthread_mutex_lock(&state_mu);
  asset_server_ready = 1;
  pthread_cond_broadcast(&state_cv);
  pthread_mutex_unlock(&state_mu);

  while (!is_client_failed()) {
    int fd = accept(listen_fd, NULL, NULL);
    if (fd < 0) {
      if (errno == EINTR) {
        continue;
      }
      if (!is_client_failed()) {
        mark_failed("accept");
      }
      break;
    }

    handle_asset_client(fd);
    close(fd);
  }

  close(listen_fd);
  return NULL;
}

static int run_start_workflow(void) {
  char projname[128];
  char *response;
  char *sid;
  char *command;
  char *winprofile;
  char *userprofile;
  char *body;
  int ok;

  ok = -1;
  response = NULL;
  sid = NULL;
  command = NULL;
  winprofile = NULL;
  userprofile = NULL;
  body = NULL;

  if (http_request("GET", "/server/index.html", NULL, &response) < 0) {
    goto out;
  }
  debug_write_file("index.html", response);
  sid = extract_input_value(response, "sid");
  free(response);
  response = NULL;
  if (sid == NULL || sid[0] == '\0') {
    debug_log("missing sid in index page");
    errno = EPROTO;
    goto out;
  }

  snprintf(projname, sizeof(projname), "benchamel-%ld", (long)getpid());

  if (http_request("GET", "/server/step2.html", NULL, &response) < 0) {
    goto out;
  }
  debug_write_file("step2.html", response);
  free(response);
  response = NULL;

  body = build_step2_body(sid, projname);
  if (http_request("POST", "/server/step3.html", body, &response) < 0) {
    goto out;
  }
  free(body);
  body = NULL;
  free(response);
  response = NULL;

  if (http_request("GET", "/server/step3.html", NULL, &response) < 0) {
    goto out;
  }
  debug_write_file("step3.html", response);
  free(response);
  response = NULL;

  body = build_step3_body(sid, projname);
  if (http_request("POST", "/server/step4.html", body, &response) < 0) {
    goto out;
  }
  free(body);
  body = NULL;
  free(response);
  response = NULL;

  if (http_request("GET", "/server/step4.html", NULL, &response) < 0) {
    goto out;
  }
  debug_write_file("step4.html", response);

  command = extract_textarea_value(response, "command");
  winprofile = extract_textarea_value(response, "winprofile");
  userprofile = extract_textarea_value(response, "userprofile");
  debug_log("step4 command=%s winprofile=%s userprofile=%s",
            (command != NULL && command[0] != '\0') ? "yes" : "no",
            (winprofile != NULL && winprofile[0] != '\0') ? "yes" : "no",
            (userprofile != NULL && userprofile[0] != '\0') ? "yes" : "no");
  free(response);
  response = NULL;
  if (command == NULL || winprofile == NULL || userprofile == NULL ||
      command[0] == '\0') {
    debug_log("step4 parsing failed: command=%p winprofile=%p userprofile=%p",
              (void *)command, (void *)winprofile, (void *)userprofile);
    errno = EPROTO;
    goto out;
  }

  body = build_start_body(sid, projname, command, winprofile, userprofile);
  if (http_request("POST", "/server/step4.html", body, &response) < 0) {
    goto out;
  }
  free(body);
  body = NULL;
  free(response);
  response = NULL;

  debug_log("posted start request for project %s", projname);
  notify_start_request(sid);
  ok = 0;

out:
  if (ok != 0) {
    debug_log("start workflow failed");
  }
  free(body);
  free(response);
  free(sid);
  free(command);
  free(winprofile);
  free(userprofile);
  return ok;
}

static void *controller_worker(void *arg) {
  (void)arg;

  wait_for_asset_server();
  if (!is_client_failed() && run_start_workflow() < 0) {
    mark_failed("httrack workflow");
  }

  usleep(HTTRACK_CLIENT_SETTLE_USEC);
  finish_worker(is_client_failed());
  return NULL;
}

static void *cancel_worker(void *arg) {
  char sid[128];
  int i;

  (void)arg;
  sid[0] = '\0';

  wait_for_start_request(sid, sizeof(sid));
  if (sid[0] == '\0') {
    finish_worker(1);
    return NULL;
  }

  for (i = 0; i < HTTRACK_CLIENT_CANCEL_TRIES && !is_client_failed(); ++i) {
    char *refresh;
    char *body;
    char *response;
    char *refresh_sid;
    char *cancel_command;

    refresh = NULL;
    response = NULL;
    refresh_sid = NULL;
    cancel_command = NULL;

    if (http_request("GET", "/server/refresh.html", NULL, &refresh) < 0) {
      mark_failed("refresh");
      free(refresh);
      break;
    }

    refresh_sid = extract_input_value(refresh, "sid");
    cancel_command = extract_cancel_file_command(refresh);
    if (i == 0) {
      debug_write_file("refresh.html", refresh);
    }
    free(refresh);

    if (refresh_sid != NULL && refresh_sid[0] != '\0') {
      strncpy(sid, refresh_sid, sizeof(sid) - 1);
      sid[sizeof(sid) - 1] = '\0';
    }

    if (cancel_command != NULL && cancel_command[0] != '\0') {
      debug_log("found cancel command: %s", cancel_command);
      body = build_cancel_file_body(sid, cancel_command);
      if (http_request("POST", "/server/step4.html", body, &response) < 0) {
        free(body);
        free(response);
        free(refresh_sid);
        free(cancel_command);
        mark_failed("cancel-file");
        break;
      }
      free(body);
      free(response);
      response = NULL;
    } else if (i > (HTTRACK_CLIENT_CANCEL_TRIES / 2)) {
      body = build_cancel_body(sid);
      if (http_request("POST", "/server/step4.html", body, &response) < 0) {
        free(body);
        free(response);
        free(refresh_sid);
        free(cancel_command);
        mark_failed("cancel");
        break;
      }
      free(body);
      free(response);
      response = NULL;
    }

    free(refresh_sid);
    free(cancel_command);
    usleep(HTTRACK_CLIENT_CANCEL_DELAY_USEC);
  }

  debug_log("cancel worker finished without triggering client failure");

  finish_worker(is_client_failed());
  return NULL;
}

void client_run(int port) {
  pthread_t asset_thread;
  pthread_t thread;
  int i;
  int err;

  debug_log("client_run port=%d", port);
  debug_write_file("client-start.txt", "client_run\n");

  signal(SIGPIPE, SIG_IGN);

  client_port = port;
  client_failed = 0;
  finished_workers = 0;
  asset_server_ready = 0;
  start_request_ready = 0;
  start_request_sid[0] = '\0';

  err = pthread_create(&asset_thread, NULL, asset_server_thread, NULL);
  if (err != 0) {
    errno = err;
    perror("pthread_create");
    abort();
  }
  pthread_detach(asset_thread);

  err = pthread_create(&thread, NULL, controller_worker, NULL);
  if (err != 0) {
    errno = err;
    perror("pthread_create");
    abort();
  }
  pthread_detach(thread);

  for (i = 0; i < HTTRACK_CLIENT_CANCEL_THREADS; ++i) {
    err = pthread_create(&thread, NULL, cancel_worker, NULL);
    if (err != 0) {
      errno = err;
      perror("pthread_create");
      abort();
    }
    pthread_detach(thread);
  }
}
