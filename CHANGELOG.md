# [unreleased]

## Fixed
- ConVul/2011-2183: thread routines return 0
- ConVul/2011-2183: `LIST_HEAD_INIT` type cast
- ConVul/2013-1792: thread routines return 0
- ConVul/2013-1792: replace `malloc.h` with `stdlib.h`
- ConVul/2015-7550: conditionally define `EOPNOTSUPP`
- ConVul/2015-7550: thread routines return 0
- ConVul/2015-7550: replace `malloc.h` with `stdlib.h`
- ConVul/2017-15265: thread routines return 0
- ConVul/2017-15265: do not include `malloc.h`
- ConVul/2017-6346: thread routine returns 0
- ConVul/2017-6346: do not include `malloc.h`
- ConVul/2016-9806: thread routine returns 0
- ConVul/2016-9806: conditionally define `offsetof`
- ConVul/2016-7911: thread routines return 0
- ConVul/2016-1973: thread routines return 0
- ConVul/2009-3547: thread routines return 0
- ConVul/2016-1972: thread routine returns 0
- ConVul/2016-1972: fix `printf` format string
- ConVul/2011-2183: enable the bug by uncommenting `hlist_del()` and `list_del()` in `scan_get_next_rmap_item()`
- ConVul/2016-9806: enable the bug by commenting the `MUTEX_FOR_CORRECT_EXE_SEQUENCE` define
- ConVul/2017-6346: enable the bug by commenting the `MUTEX_FOR_CORRECT_EXE_SEQUENCE` define
- ConVul/2017-15265: enable the bug by commenting the `MUTEX_FOR_CORRECT_EXE_SEQUENCE` define
- sctbench/chess: space between consecutive right angle brackets
- sctbench/chess: add intermediate cast to `uintptr_t`
- sctbench/chess: disable deprecation warning for std::strstream
- sctbench/pbzip2.cpp: fix `printf` format string
- sctbench/stringbuffer-jdk: cast string literal as `char *`
- sctbench/stringbuffer-jdk: thread routine returns ww0
- sctbench/concurrent-software-benchmarks/reorder_3_bad.c: replaced with a version of `reorder_bad.c.skip` with `iSet` set to 2
- sctbench/concurrent-software-benchmarks/reorder_4_bad.c: replaced with a version of `reorder_bad.c.skip` with `iSet` set to 3
- sctbench/concurrent-software-benchmarks/reorder_5_bad.c: replaced with a version of `reorder_bad.c.skip` with `iSet` set to 4
- sctbench/concurrent-software-benchmarks/reorder_10_bad.c: replaced with a copy of `reorder_bad.c.skip`
- sctbench/concurrent-software-benchmarks/reorder_20_bad.c: replaced with a version of `reorder_bad.c.skip` with `iSet` and `iCheck` set to 10
- sctbench/concurrent-software-benchmarks/twostage_100_bad.c: replaced with a version of `twostage_bad.c` with `iTThreads` set to 99
- sctbench/concurrent-software-benchmarks/wronglock_3_bad.c: replaced with a version of `wronglock_bad.c` with `iNum2` set to 3
- sctbench/concurrent-software-benchmarks/account_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/account_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/arithmetic_prog_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/arithmetic_prog_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/bluetooth_driver_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/carter01_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/circular_buffer_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/circular_buffer_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil2_sat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil2_unsat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil3_sat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil3_unsat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil4_sat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil4_unsat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil5_sat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil5_unsat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil6_sat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil6_unsat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil7_sat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/din_phil7_unsat.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/fanger01_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/indexer_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/lazy01_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/lazy01_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/micro_10_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/micro_2_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/micro_3_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/phase01_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/phase01_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/stack_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/stack_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/stateful01_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/stateful06_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/stateful20_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/sync01_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/sync01_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/sync02_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/sync02_ok.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/token_ring_bad.c: thread routines return 0
- sctbench/concurrent-software-benchmarks/account_bad.c: add `pthread_join()`
- sctbench/concurrent-software-benchmarks/account_ok.c: add `pthread_join()`
- sctbench/concurrent-software-benchmarks/micro_10_ok.c: add `pthread_join()`
- sctbench/concurrent-software-benchmarks/micro_2_ok.c: add `pthread_join()`
- sctbench/concurrent-software-benchmarks/micro_3_ok.c: add `pthread_join()`
- sctbench/concurrent-software-benchmarks/token_ring_bad.c: add `pthread_join()`

## Added
- build action for macOS

# [0.1] - 2025-03-07

## Added

- ConVul
- sctbench/chess
- libvsync v4.1.0 - SVCOMP bugs
- GitHub action with Ubuntu 22.04 and 24.04
- Simple benchkit campaign

## Fixed

- ConVul/2016-9806: `printf` of `pthread_self`
- ConVul/2017-6346: `posix_memalign` instead of `memalign`
- ConVul/2017-15265: `posix_memalign` instead of `memalign`
