# [unreleased]

## Fixed
- convul/2011-2183: thread routines return 0
- convul/2011-2183: `LIST_HEAD_INIT` type cast
- convul/2013-1792: thread routines return 0
- convul/2013-1792: replace `malloc.h` with `stdlib.h`
- convul/2015-7550: conditionally define `EOPNOTSUPP`
- convul/2015-7550: thread routines return 0
- convul/2015-7550: replace `malloc.h` with `stdlib.h`
- convul/2017-15265: thread routines return 0
- convul/2017-15265: do not include `malloc.h`
- convul/2017-6346: thread routine returns 0
- convul/2017-6346: do not include `malloc.h`
- convul/2016-9806: thread routine returns 0
- convul/2016-9806: conditionally define `offsetof`
- convul/2016-7911: thread routines return 0
- convul/2016-1973: thread routines return 0
- convul/2009-3547: thread routines return 0
- convul/2016-1972: thread routine returns 0
- convul/2016-1972: fix `printf` format string
- sctbench/chess: space between consecutive right angle brackets
- sctbench/chess: add intermediate cast to `uintptr_t`
- sctbench/chess: disable deprecation warning for std::strstream
- sctbench/pbzip2.cpp: fix `printf` format string
- sctbench/stringbuffer-jdk: cast string literal as `char *`
- sctbench/stringbuffer-jdk: thread routine returns ww0

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

- convul/2016-9806: `printf` of `pthread_self`
- convul/2017-6346: `posix_memalign` instead of `memalign`
- convul/2017-15265: `posix_memalign` instead of `memalign`
