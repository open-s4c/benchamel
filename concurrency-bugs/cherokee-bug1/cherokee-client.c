#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
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
static const char *const k_client_request =
    "GET /index.html HTTP/1.0\r\n"
    "Host: 127.0.0.1\r\n"
    "If-Modified-Since: Sat Oct 1994 19:43:31 GMT\r\n"
    "Connection: close\r\n"
    "\r\n";
static const char *const k_warning_marker = "Warning: Unparseable time";
static const char *const k_access_marker = "\"GET /index.html HTTP/1.0\"";

static int client_failed = 0;
static int finished_workers = 0;

static void mark_failed(const char *what) {
  if (__sync_bool_compare_and_swap(&client_failed, 0, 1)) {
    perror(what);
  }
}

static int is_client_failed(void) { return __sync_fetch_and_add(&client_failed, 0); }

static void clear_access_log(void) {
  FILE *fp;

  fp = fopen(BENCHAMEL_CHEROKEE_BUG1_ACCESS_LOG, "w");
  if (fp == NULL) {
    perror("fopen");
    abort();
  }

  fclose(fp);
}

static int access_log_has_corruption(void) {
  char line[8192];
  FILE *fp;

  fp = fopen(BENCHAMEL_CHEROKEE_BUG1_ACCESS_LOG, "r");
  if (fp == NULL) {
    perror("fopen");
    abort();
  }

  while (fgets(line, sizeof(line), fp) != NULL) {
    if (strstr(line, k_warning_marker) != NULL &&
        strstr(line, k_access_marker) != NULL) {
      fclose(fp);
      return 1;
    }
  }

  fclose(fp);
  return 0;
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
  (void)arg;

  if (wait_for_server_ready() < 0) {
    mark_failed("connect");
    finish_worker(/*failed=*/1);
    return NULL;
  }

  for (i = 0; i < CHEROKEE_CLIENT_LOOPS && !is_client_failed(); ++i) {
    int fd = connect_with_retry(CHEROKEE_CLIENT_CONNECT_TRIES);
    if (fd < 0) {
      mark_failed("connect");
      break;
    }

    if (send_all(fd, k_client_request, strlen(k_client_request)) < 0) {
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
    int err = pthread_create(&thread, NULL, client_worker, NULL);
    if (err != 0) {
      errno = err;
      perror("pthread_create");
      abort();
    }
    pthread_detach(thread);
  }
}
