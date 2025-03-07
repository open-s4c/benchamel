SPDX-FileCopyrightText: 2024 Huawei Technologies Co., Ltd.
SPDX-License-Identifier: MIT

# libvsync benchmarks

This directory contains real-world concurrency-related benchmarks based on
a subset of [libvsync][], a verified library of synchronization primitives
and concurrent data structures. With these benchmarks we hope to reduce the
gap between academic tools and industry needs.

Note that the orignal code has been verified and optimized for weak memory
models. In this directory, however, two modifications have been made:

1. all atomic accesses are upgraded to be **sequentially consistent**,
2. some algorithms have **injected bugs** that cause safety violations.

These bugs are part of the SVCOMP benchmarks:

  https://gitlab.com/sosy-lab/benchmarking/sv-benchmarks

For the full library without injected bugs, access:

  https://github.com/open-s4c/libvsync

[Reference][paper]: VSync: push-button verification and optimization
for synchronization primitives on weak memory models - Jonas Oberhauser,
Rafael Lourenco de Lima Chehab, Diogo Behrens, Ming Fu, Antonio Paolillo,
Lilith Oberhauser, Koustubha Bhat, Yuzhong Wen, Haibo Chen, Jaeho Kim,
Viktor Vafeiadis - ASPLOS 2021.

[Contributed by][drc]: Huawei Dresden Research Center

[drc]: https://github.com/open-s4c
[paper]: https://dl.acm.org/doi/10.1145/3445814.3446748
[libvsync]: https://github.com/open-s4c/libvsync

## Test cases

The files in this directory are organized as follows:

- `include/`: libvsync with bugs
- `test/`: selected test cases
- `bugs.patch`: patch of injected bugs

There are mainly two groups of test cases here: bounded queue test cases
(3 cases), spinlock test cases (the rest).

**The spinlock test cases** all follow the same pattern. Each thread acquires
a lock, increments a variable x and a variable y, and releases the lock. At the
end of the execution (once the threads join), the x and y are compared with
the expected value which is dependent on the number of threads running. To
increase the chance of finding bugs, there are some additional complications
added to the test cases. First, in most of them a subset of the threads does
the job twice (ie, acquire, increment, release). Second, for reader-writer
locks, there are additional reader threads that also check whether x and y
are the same in their reader critical sections.

**The bounded queue test cases** are specialized to the particular
queue implementation. There are two lock-less implementations:
single-producer/single-consumer (SPSC) and multiple-producer/multiple-consumer
(MPMC). The tests mainly start a number of producer and consumer threads and
check whether items are lost, duplicated or (for SPSC) dequeued in FIFO order.

The test cases are in `test` and some of them use helper header files in
`include/test`. The spinlock and queue implementations are purely header
implementations and are in `include/vsync`. The algorithms employ a library
of atomics called `vatomic` that is also in `include/vsync/atomic`. For
verification purposes, when the test cases are expanded the `vatomic`
interface maps to compiler builtins that are accepted in GCC and Clang
(`__atomic_...`).

## Correctness

To the best of our knowledge, all algorithms/test cases are correct: they
do not cause safety violations and do not hang -- even on weak memory model
systems. Over the course of 2+ years, we have carefully and gradually verified
these algorithms with model checkers to guarantee their correctness.

For the sake of the SVCOMP competition, we have injected bugs in a few of these
algorithms. The bugs are all contained in `bugs.patch` along with a comment
explaining the reasoning behind (look for comments with `/* BUG: ...` ).

During our efforts to verify these algorithms one of the major issues was that
existing model checkers often provide a relatively poor support to real code,
eg, they often do not understand real atomic operations from builtins and
instead rely on `__VERIFIER__` fake operations; they sometimes cannot handle
unbounded loops; and many other little things that are done in practice but
not considered when writing a model checker.

Our intent with this set of benchmarks is to make the community aware of
the kind of **concurrent code** that is used in practice and, with that,
improve the quality and impact of the available tools.
