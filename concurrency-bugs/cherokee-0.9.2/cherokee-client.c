#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cherokee-client.h"

enum {
  CHEROKEE_CLIENT_THREADS = 2,
  CHEROKEE_CLIENT_LOOPS = 50,
  CHEROKEE_CLIENT_PORT = 50081,
  CHEROKEE_CLIENT_STARTUP_TRIES = 400,
  CHEROKEE_CLIENT_CONNECT_TRIES = 100,
  CHEROKEE_CLIENT_RETRY_USEC = 10000,
  CHEROKEE_CLIENT_BUFFER_SIZE = 4096,
  CHEROKEE_CLIENT_SOCKET_TIMEOUT_SEC = 1,
};

static const char *const k_client_host = "127.0.0.1";
static const char *const k_request_paths[CHEROKEE_CLIENT_THREADS] = {
    "/images/powered_by_cherokee.png",
    "/images/cherokee-logo.png",
};

static int client_failed = 0;
static int finished_workers = 0;

static void mark_failed(const char *what) {
  if (__sync_bool_compare_and_swap(&client_failed, 0, 1)) {
    perror(what);
  }
}

static int is_client_failed(void) { return __sync_fetch_and_add(&client_failed, 0); }

static void clear_access_log(void) {
  FILE *fp = fopen(BENCHAMEL_CHEROKEE_092_ACCESS_LOG, "w");

  if (fp == NULL) {
    perror("fopen");
    abort();
  }

  fclose(fp);
}

static int is_valid_month(const char *month) {
  static const char *const months[] = {"Jan", "Feb", "Mar", "Apr",
                                       "May", "Jun", "Jul", "Aug",
                                       "Sep", "Oct", "Nov", "Dec"};
  int i;

  for (i = 0; i < 12; ++i) {
    if (strncmp(month, months[i], 3) == 0) {
      return 1;
    }
  }

  return 0;
}

static int line_has_valid_timestamp(const char *line) {
  const char *open = strstr(line, " [");
  const char *close;
  int day;
  int year;
  int hour;
  int minute;
  int second;
  int tz_hour;
  int tz_minute;
  char month[4];
  char sign;
  int parsed;

  if (open == NULL) {
    return 0;
  }

  close = strchr(open + 2, ']');
  if (close == NULL || close[1] != ' ' || close[2] != '"') {
    return 0;
  }

  month[0] = '\0';
  parsed = sscanf(open + 2, "%2d/%3[A-Za-z]/%4d:%2d:%2d:%2d %c%2d%2d", &day,
                  month, &year, &hour, &minute, &second, &sign, &tz_hour,
                  &tz_minute);
  if (parsed != 9) {
    return 0;
  }

  if (day < 1 || day > 31 || year < 1970 || hour < 0 || hour > 23 ||
      minute < 0 || minute > 59 || second < 0 || second > 60 ||
      (sign != '+' && sign != '-') || tz_hour < 0 || tz_hour > 23 ||
      tz_minute < 0 || tz_minute > 59) {
    return 0;
  }

  return is_valid_month(month);
}

static int line_has_valid_request(const char *line) {
  int path0 = strstr(line, k_request_paths[0]) != NULL;
  int path1 = strstr(line, k_request_paths[1]) != NULL;

  if (path0 == path1) {
    return 0;
  }

  if (strstr(line, "\"GET ") == NULL || strstr(line, " HTTP/1.0\" ") == NULL) {
    return 0;
  }

  return 1;
}

static int access_log_has_corruption(void) {
  char line[8192];
  FILE *fp = fopen(BENCHAMEL_CHEROKEE_092_ACCESS_LOG, "r");
  int saw_request = 0;

  if (fp == NULL) {
    perror("fopen");
    abort();
  }

  while (fgets(line, sizeof(line), fp) != NULL) {
    if (line[0] == '\0' || line[0] == '\n') {
      continue;
    }

    saw_request = 1;
    if (!line_has_valid_timestamp(line) || !line_has_valid_request(line)) {
      fclose(fp);
      return 1;
    }
  }

  fclose(fp);
  return !saw_request;
}

static int connect_once(void) {
  int fd;
  struct sockaddr_in addr;
  struct timeval timeout;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  timeout.tv_sec = CHEROKEE_CLIENT_SOCKET_TIMEOUT_SEC;
  timeout.tv_usec = 0;
  (void)setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(CHEROKEE_CLIENT_PORT);
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
    usleep(CHEROKEE_CLIENT_RETRY_USEC);
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

static int drain_socket(int fd) {
  char buf[CHEROKEE_CLIENT_BUFFER_SIZE];

  while (1) {
    ssize_t read_bytes = recv(fd, buf, sizeof(buf), 0);
    if (read_bytes > 0) {
      continue;
    }
    if (read_bytes == 0) {
      return 0;
    }
    if (errno == EINTR) {
      continue;
    }
    return -1;
  }
}

static int wait_for_server_ready(void) {
  int fd = connect_with_retry(CHEROKEE_CLIENT_STARTUP_TRIES);

  if (fd < 0) {
    return -1;
  }

  close(fd);
  return 0;
}

static void finish_worker(int failed) {
  int should_exit = 0;
  int should_abort = 0;
  int bug_detected = 0;
  int finished;

  if (failed) {
    (void)__sync_bool_compare_and_swap(&client_failed, 0, 1);
  }

  finished = __sync_add_and_fetch(&finished_workers, 1);
  if (finished == CHEROKEE_CLIENT_THREADS) {
    should_abort = is_client_failed();
    if (!should_abort) {
      usleep(250000);
      bug_detected = access_log_has_corruption();
      should_abort = bug_detected;
    }
    should_exit = !should_abort;
  }

  if (should_abort) {
    if (bug_detected) {
      fprintf(stderr, "bug detected: access log corruption\n");
      fflush(stderr);
    }
    abort();
  }

  if (should_exit) {
    exit(0);
  }
}

static void *client_worker(void *arg) {
  int i;
  intptr_t index = (intptr_t)arg;
  char request[512];

  if (wait_for_server_ready() < 0) {
    mark_failed("connect");
    finish_worker(/*failed=*/1);
    return NULL;
  }

  snprintf(request, sizeof(request),
           "GET %s HTTP/1.0\r\n"
           "Host: 127.0.0.1\r\n"
           "Connection: close\r\n"
           "\r\n",
           k_request_paths[index]);

  for (i = 0; i < CHEROKEE_CLIENT_LOOPS && !is_client_failed(); ++i) {
    int fd = connect_with_retry(CHEROKEE_CLIENT_CONNECT_TRIES);
    if (fd < 0) {
      mark_failed("connect");
      break;
    }

    if (send_all(fd, request, strlen(request)) < 0) {
      close(fd);
      mark_failed("send");
      break;
    }

    shutdown(fd, SHUT_WR);

    if (drain_socket(fd) < 0) {
      close(fd);
      mark_failed("recv");
      break;
    }

    close(fd);
  }

  finish_worker(/*failed=*/is_client_failed());
  return NULL;
}

void client_run(void) {
  int i;

  finished_workers = 0;
  client_failed = 0;
  clear_access_log();

  for (i = 0; i < CHEROKEE_CLIENT_THREADS; ++i) {
    pthread_t thread;
    int err = pthread_create(&thread, NULL, client_worker, (void *)(intptr_t)i);
    if (err != 0) {
      errno = err;
      perror("pthread_create");
      abort();
    }
    pthread_detach(thread);
  }
}
