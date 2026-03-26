#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"

const char*   g_host    = "127.0.0.1";
unsigned short g_port   = 11211;
const char*   g_key     = "test";
size_t        g_key_len = 4;
unsigned int  g_loop    = 1;
unsigned int  g_threads = 2;
unsigned int  g_step    = 10;

static int mc_connect(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(g_port);
    if (inet_pton(AF_INET, g_host, &addr.sin_addr) != 1) {
        fprintf(stderr, "invalid host: %s\n", g_host);
        exit(1);
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); exit(1);
    }
    return fd;
}

static void send_all(int fd, const char* buf, size_t len)
{
    while (len > 0) {
        ssize_t n = send(fd, buf, len, 0);
        if (n <= 0) { perror("send"); exit(1); }
        buf += n;
        len -= (size_t)n;
    }
}

static void recv_line(int fd, char* buf, size_t buflen)
{
    size_t i = 0;
    while (i < buflen - 1) {
        ssize_t n = recv(fd, buf + i, 1, 0);
        if (n <= 0) { perror("recv"); exit(1); }
        i++;
        if (buf[i - 1] == '\n') break;
    }
    buf[i] = '\0';
}

static void recv_exact(int fd, char* buf, size_t len)
{
    while (len > 0) {
        ssize_t n = recv(fd, buf, len, 0);
        if (n <= 0) { perror("recv"); exit(1); }
        buf += n;
        len -= (size_t)n;
    }
}

static void mc_set(int fd, const char* key, const char* value)
{
    char cmd[512];
    size_t vlen = strlen(value);
    int n = snprintf(cmd, sizeof(cmd), "set %s 0 0 %zu\r\n%s\r\n", key, vlen, value);
    send_all(fd, cmd, (size_t)n);

    char resp[64];
    recv_line(fd, resp, sizeof(resp));
    if (strncmp(resp, "STORED", 6) != 0) {
        fprintf(stderr, "set failed: %s", resp);
        exit(1);
    }
}

static void mc_incr(int fd, const char* key, unsigned int step)
{
    char cmd[256];
    int n = snprintf(cmd, sizeof(cmd), "incr %s %u\r\n", key, step);
    send_all(fd, cmd, (size_t)n);

    char resp[64];
    recv_line(fd, resp, sizeof(resp));
    if (strncmp(resp, "NOT_FOUND", 9) == 0) {
        fprintf(stderr, "incr: key not found\n");
        exit(1);
    }
}

static int mc_get(int fd, const char* key,
                  char* valbuf, size_t valbuflen)
{
    char cmd[256];
    int n = snprintf(cmd, sizeof(cmd), "get %s\r\n", key);
    send_all(fd, cmd, (size_t)n);

    char line[256];
    recv_line(fd, line, sizeof(line));

    if (strncmp(line, "END", 3) == 0) return -1; /* NOT_FOUND */

    if (strncmp(line, "VALUE", 5) != 0) {
        fprintf(stderr, "get unexpected response: %s", line);
        exit(1);
    }

    char rkey[128];
    uint32_t flags;
    size_t bytes;
    if (sscanf(line, "VALUE %127s %u %zu", rkey, &flags, &bytes) != 3) {
        fprintf(stderr, "get: parse error: %s", line);
        exit(1);
    }

    size_t to_copy = (bytes < valbuflen - 1) ? bytes : valbuflen - 1;
    recv_exact(fd, valbuf, bytes);
    valbuf[to_copy] = '\0';

    recv_exact(fd, line, 2);
    recv_line(fd, line, sizeof(line));

    return (int)to_copy;
}

static void* worker(void* arg)
{
    (void)arg;
    int fd = mc_connect();
    for (unsigned int i = 0; i < g_loop; ++i) {
        mc_incr(fd, g_key, g_step);
    }
    close(fd);
    return NULL;
}

pthread_t* threads = NULL;
pthread_t client;

extern struct event_base *main_base;

static void *
client_func(void *arg)
{
    (void)arg;
    client_start();
    client_join();
    return NULL;
}

void 
client_run(void)
{
    pthread_create(&client, NULL, client_func, NULL);
    pthread_detach(client);
}

void
client_start(void)
{
    {
        int fd = mc_connect();
        mc_set(fd, g_key, "0");
        close(fd);
    }
    
    threads = malloc(g_threads * sizeof(pthread_t));
    if (!threads) { perror("malloc"); exit(1); }
    
    for (unsigned int i = 0; i < g_threads; ++i) {
        int err = pthread_create(&threads[i], NULL, worker, NULL);
        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            exit(1);
        }
    }
}

void 
client_join(void)
{
    for (unsigned int i = 0; i < g_threads; ++i) {
        void* ret;
        int err = pthread_join(threads[i], &ret);
        if (err != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(err));
        }
    }
    free(threads);
    
    {
        char value[64];
        int fd = mc_connect();
        if (mc_get(fd, g_key, value, sizeof(value)) < 0) {
            fprintf(stderr, "key not found after incr\n");
            exit(1);
        }
        close(fd);

        int expected = (int)(g_threads * g_loop * g_step);
        if (expected != atoi(value)) {
            fprintf(stderr, "expected: %d, actual: %d\n", expected, atoi(value));
            abort();
        }
        assert(expected == atoi(value));
        /* printf("expected: %d\n", expected); */
        /* printf("result:   %s\n", value); */
    }

    exit(0);
}
