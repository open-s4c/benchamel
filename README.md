# Benchamel - A curated collection of concurrency bugs

The collection of bugs in this repository form a benchmark suite of the
effectiveness and efficiency of concurrency testing tools.

Project goals are:
- **Maintainance**: The buggy applications come from diverse benchmarks. Many
  of them cannot be compiled with recent compilers and operating systems. We
  aim at providing a unified way (via CMake) to compile all these different
  examples.

- **Parameterized evaluation**: We plan to add
  [benchkits](https://github.com/open-s4c/benchkit) for the different
  applications, exposing the parameters to benchkit campaigns. At the moment we
  only have a [plain campaign](TBD) for the merged applications.

# Sources

- [SCTBench](https://github.com/mc-imperial/sctbench.git)
- [ConVul](https://github.com/mryancai/ConVul)
- [libvsync](https://github.com/open-s4c/libvsync)
- [PERIOD](https://github.com/wcventure/PERIOD)
- [Jieyu's concurrency-bugs](https://github.com/jieyu/concurrency-bugs)

Each directory contains a LICENSE and a VERSION file indicating the respective
license, software version and URL.

# ThreadSanitizer

To build the benchmarks with ThreadSanitizer enabled, configure CMake with:

```sh
cmake -S . -B build -DBENCHAMEL_BUILD_TSAN=ON
```

This adds `-fsanitize=thread -g -fno-omit-frame-pointer` to benchmark targets.
When a benchmark builds downloaded dependencies, the same sanitizer flags
should also be propagated into those dependencies so the benchmark and its
dependencies are built consistently. For example, `memcached-127` propagates
them into its external `libevent` build.

# FetchContent Cache

Some benchmarks download third-party source archives with `FetchContent`.

If you want CMake to stay offline and reuse a previously downloaded archive in
the same build tree, configure with:

```sh
cmake -S . -B build -DFETCHCONTENT_FULLY_DISCONNECTED=ON
```

If you already unpacked a dependency yourself, you can also bypass the
download entirely by pointing `FetchContent` at that extracted source tree with
`FETCHCONTENT_SOURCE_DIR_<NAME>`, where `<NAME>` is the dependency name from
`FetchContent_Declare(...)` converted to uppercase.

For example, Jieyu's `concurrency-bugs` archive is declared as `JCBUGS`, so
you can configure with:

```sh
cmake -S . -B build \
  -DFETCHCONTENT_SOURCE_DIR_JCBUGS=/path/to/concurrency-bugs-master
```

# License

Most subdirectories contain the license file under which the contents of the
directory are distributed. Other software components in this repository are
released under the MIT license (see [LICENSE](LICENSE)).
