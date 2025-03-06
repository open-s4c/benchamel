/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */

#define NTHREADS 3

#include <vsync/spinlock/hemlock.h>
#include <test/boilerplate/lock.h>

hemlock_t lock1 = HEMLOCK_INIT();
hemlock_t lock2 = HEMLOCK_INIT();
struct hem_node_s nodes[NTHREADS];


/* This test case is fine with the correct implementation of hemlock, but it
 * fails when we inject the bug (see hemlock.h). Example:
 *
 * - Thread 1 acquires lock2 and acquires lock1
 * - Thread 0 blocks at lock1
 * - Thread 2 blocks at lock2
 * - Thread 1 releases lock1.
 *
 * In the last step, due to the injected bug, thread 2 also thinks lock2 was
 * released and enters the critical section together with thread 1.
 */
void
acquire(vuint32_t tid)
{
    if (tid == 0) {
        hemlock_acquire(&lock1, &nodes[tid]);
        hemlock_release(&lock1, &nodes[tid]);
    }

    hemlock_acquire(&lock2, &nodes[tid]);

    if (tid == 1) {
        hemlock_acquire(&lock1, &nodes[tid]);
        hemlock_release(&lock1, &nodes[tid]);
    }
}

void
release(vuint32_t tid)
{
    hemlock_release(&lock2, &nodes[tid]);
}
