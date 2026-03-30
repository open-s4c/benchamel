/*
 * Copyright (C) 2025-2026 Huawei Technologies Co., Ltd.
 * SPDX-License-Identifier: MIT
 */
#define VSKIPLIST_MAX_HEIGHT 4
#define SMR_MAX_NTHREADS 4
#define NTHREADS 3
#define INIT_SIZE 3
#define SMR_NONE

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vsync/map/skiplist_lf.h>
#include <vsync/vtypes.h>

/* keep track of the allocation counts */
static struct {
  uint64_t allocs;
  uint64_t frees;
} counts;

static void *t_malloc(size_t size) {
  void *ptr = malloc(size);

  ASSERT(ptr && "allocation failed");
  __atomic_add_fetch(&counts.allocs, 1, __ATOMIC_RELAXED);
  return ptr;
}

static void t_free(void *ptr) {
  if (ptr != NULL) {
    __atomic_add_fetch(&counts.frees, 1, __ATOMIC_RELAXED);
  }
  free(ptr);
}

static bool t_no_leak(void) {
  return __atomic_load_n(&counts.allocs, __ATOMIC_RELAXED) ==
         __atomic_load_n(&counts.frees, __ATOMIC_RELAXED);
}

typedef enum { NO_EXPECTATION, SUCC_EXPECTED, FAIL_EXPECTED } expectation_t;

typedef struct skiplist_mock_node_s {
  vskiplist_key_t key;
  vskiplist_node_t skip_node; /* must be here */
} skiplist_mock_node_t;

typedef struct retired_node_s {
  skiplist_mock_node_t *node;
  struct retired_node_s *next;
} retired_node_t;

static vskiplist_node_t *g_head = NULL;
static vskiplist_node_t *g_tail = NULL;
static vskiplist_t g_skiplist;
static retired_node_t *g_retired_nodes = NULL;

static const vskiplist_key_t g_init_keys[INIT_SIZE] = {3, 5, 6};

static bool g_t0_add_4_succeeded = false;
static bool g_t1_rem_5_succeeded = false;

static skiplist_mock_node_t *node_from_skip_node(vskiplist_node_t *skip_node) {
  if (skip_node == NULL) {
    return NULL;
  }
  return V_CONTAINER_OF(skip_node, skiplist_mock_node_t, skip_node);
}

static void print_node(vskiplist_node_t *skip_node, bool marked, void *args) {
  skiplist_mock_node_t *node = node_from_skip_node(skip_node);

  ASSERT(node);
  printf("\t%s %" VUINTPTR_FORMAT "\n", marked ? "DNE" : "EXISTS", node->key);
  V_UNUSED(args);
}

static int compare_node_key(vskiplist_node_t *skip_node, vskiplist_key_t key) {
  skiplist_mock_node_t *node = node_from_skip_node(skip_node);

  ASSERT(node);
  if (node->key < key) {
    return -1;
  }
  if (node->key > key) {
    return 1;
  }
  return 0;
}

static void retire_node(vskiplist_node_t *skip_node, void *args) {
  retired_node_t *retired = t_malloc(sizeof(*retired));

  ASSERT(retired);
  retired->node = V_CONTAINER_OF(skip_node, skiplist_mock_node_t, skip_node);
  retired->next = g_retired_nodes;
  g_retired_nodes = retired;
  V_UNUSED(args);
}

static skiplist_mock_node_t *alloc_node(vskiplist_key_t key, size_t *height) {
  size_t bytes =
      vskiplist_calc_node_sz(&g_skiplist, sizeof(skiplist_mock_node_t), height);
  skiplist_mock_node_t *node = t_malloc(bytes);

  ASSERT(node);
  node->key = key;
  return node;
}

static void print_skiplist(void) {
  for (size_t level = 0; level < VSKIPLIST_MAX_HEIGHT; ++level) {
    printf("vskiplist.level[%zu]\n", level);
    _vskiplist_visit_level(&g_skiplist, level, print_node, NULL);
  }
}

static void assert_lookup(vskiplist_key_t key, expectation_t expectation) {
  vskiplist_node_t *node = vskiplist_lookup(&g_skiplist, key);

  if (expectation == SUCC_EXPECTED) {
    if (node == NULL) {
      print_skiplist();
    }
    ASSERT(node && "node is not found");
    ASSERT(compare_node_key(node, key) == 0);
    return;
  }
  if (expectation == FAIL_EXPECTED) {
    ASSERT(node == NULL);
    return;
  }
  ASSERT(0 && "lookup expectation not supported");
}

static void assert_bool_matches(bool value, expectation_t expectation) {
  if (expectation == NO_EXPECTATION) {
    return;
  }
  if (expectation == SUCC_EXPECTED) {
    ASSERT(value);
    return;
  }
  if (expectation == FAIL_EXPECTED) {
    ASSERT(!value);
    return;
  }
  ASSERT(0 && "expectation not supported");
}

static void insert_initial_key(vskiplist_key_t key) {
  size_t height = 0;
  vskiplist_node_t *existing_node = NULL;
  skiplist_mock_node_t *node = alloc_node(key, &height);
  bool inserted =
      vskiplist_add(&g_skiplist, key, &node->skip_node, &existing_node, height);

  ASSERT(inserted);
  ASSERT(existing_node == NULL);
}

static void init_test(void) {
  g_head = t_malloc(VSKIPLIST_SENTINEL_SZ);
  g_tail = t_malloc(VSKIPLIST_SENTINEL_SZ);
  ASSERT(g_head);
  ASSERT(g_tail);

  vskiplist_init(&g_skiplist, compare_node_key, retire_node, NULL, time(0),
                 g_head, g_tail);

  for (size_t i = 0; i < INIT_SIZE; ++i) {
    insert_initial_key(g_init_keys[i]);
  }
}

static void destroy_test(void) {
  retired_node_t *retired = NULL;

  print_skiplist();
  vskiplist_destroy(&g_skiplist);

  while (g_retired_nodes != NULL) {
    retired = g_retired_nodes;
    g_retired_nodes = retired->next;
    t_free(retired->node);
    t_free(retired);
  }

  t_free(g_head);
  t_free(g_tail);
  g_head = NULL;
  g_tail = NULL;

  ASSERT(t_no_leak());
}

static void *t0(void *arg) {
  size_t height = 0;
  vskiplist_node_t *existing_node = NULL;
  skiplist_mock_node_t *new_node = NULL;
  V_UNUSED(arg);

  new_node = alloc_node(4, &height);
  g_t0_add_4_succeeded = vskiplist_add(&g_skiplist, 4, &new_node->skip_node,
                                       &existing_node, height);
  ASSERT(g_t0_add_4_succeeded);
  ASSERT(existing_node == NULL);
  return NULL;
}

static void *t1(void *arg) {
  V_UNUSED(arg);

  g_t1_rem_5_succeeded = vskiplist_remove(&g_skiplist, 5);
  ASSERT(g_t1_rem_5_succeeded);
  return NULL;
}

static void *t2(void *arg) {
  vskiplist_node_t *node = NULL;
  V_UNUSED(arg);

  node = vskiplist_lookup(&g_skiplist, 6);
  if (node == NULL) {
    print_skiplist();
  }
  ASSERT(node && "node is not found");
  ASSERT(compare_node_key(node, 6) == 0);
  return NULL;
}

int main(void) {
  pthread_t threads[NTHREADS];

  init_test();

  pthread_create(&threads[0], NULL, t0, (void *)0);
  pthread_create(&threads[1], NULL, t1, (void *)1);
  pthread_create(&threads[2], NULL, t2, (void *)2);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);

  assert_bool_matches(g_t0_add_4_succeeded, SUCC_EXPECTED);
  assert_bool_matches(g_t1_rem_5_succeeded, SUCC_EXPECTED);
  assert_lookup(3, SUCC_EXPECTED);
  assert_lookup(4, SUCC_EXPECTED);
  assert_lookup(5, FAIL_EXPECTED);
  assert_lookup(6, SUCC_EXPECTED);

  destroy_test();
  return 0;
}
