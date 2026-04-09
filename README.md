# Benchamel - A curated collection of concurrency bugs

The collection of bugs in this repository form a benchmark suite of the
effectiveness and efficiency of concurrency testing tools.

Project goals are:
- **Maintenance**: The buggy applications come from diverse benchmarks. Many
  of them cannot be compiled with recent compilers and operating systems. We
  aim at providing a unified way (via CMake) to compile all these different
  examples.

- **Parameterized evaluation**: We plan to add
  [benchkits](https://github.com/open-s4c/benchkit) for the different
  applications, exposing the parameters to benchkit campaigns. At the moment we
  only have a [plain campaign](TBD) for the merged applications.

- **Long-term preservation**: We want this repository to remain useful for
  future tools and for tracking how bug benchmarks evolve as toolchains,
  compilers, sanitizers, and platforms change over time.

# Benchmark Preparation Policy

Each binary in this repository is meant to demonstrate one bug in one program.
When a program has multiple bugs, the benchmark suffix is usually either the
issue number or a local bug count.

Each bug lives in its own directory, with a `CMakeLists.txt` that is
responsible for fetching, patching, and building the program and all
non-trivial dependencies needed to reproduce that bug.

Each bug directory should also contain a `README.md` describing the bug,
reproduction intent, and benchmark-specific notes. When part of that
information comes from another benchmark suite, paper, repository, or upstream
source, we keep that provenance in a `REFERENCE` file. This is intended to
replace the older `VERSION` naming used in parts of the repository.

When preparing a benchmark, the goal is to expose a canonical standalone binary
that can be executed without the user needing to know the original invocation
details of the program. In practice, we aim for the following:

- **Canonical invocation**: If the original program needs command-line
  arguments, we inject a self-`exec` path that runs the program again with the
  default arguments when launched without arguments. We also hardcode any file
  paths needed by that canonical execution.

- **No extra process creation**: Apart from that initial self-`exec` used to
  supply canonical default arguments, we try to avoid any further `exec` or
  `fork` in the benchmark execution path that we actually drive. The goal is
  for the benchmark to remain a single process unless the original bug
  fundamentally requires another model and we have not normalized it yet.

- **Embedded clients**: If the original bug requires an external client, script,
  or helper process, we replace it with the simplest possible client written in
  C, link it into the benchmark, and start it from the benchmark process using
  a thread. That client may create additional threads if the benchmark needs
  them.

- **Embedded failure detection when needed**: If the bug's effect would
  otherwise be silent, such as state corruption that is only visible by later
  inspection, we try to embed a checker into the benchmark itself, either
  through an extra `.c` file or directly in the source when the logic is small
  enough. If the bug already manifests as a direct malfunction such as a
  segmentation fault or a deadlock, that is the error indicator, i.e. we do not
  wrap it with an explicit detector.

- **Downloaded source plus documented changes**: Real program sources are
  downloaded as distfiles. Changes to those sources should be documented as a
  patch series. Some patches come from upstream benchmark suites such as
  PERIOD, SCTBench, or Jieyu's `concurrency-bugs`; others are local Benchamel
  patches. We try to record the source of each patch in the patch file itself.

- **Provided inputs**: If a benchmark needs an input file, we keep that input
  in the repository and copy it into the build tree as part of the benchmark
  build.

- **Compatibility fixes**: We also carry patches that fix warnings and
  unrelated build/runtime issues so the buggy programs remain testable with
  modern compilers and, when practical, on platforms beyond Linux. These
  changes are intended to preserve the benchmark, not to change the underlying
  bug.

- **Unified outputs**: All benchmark binaries land in `build/bin/` and use a
  `prefix.name` naming scheme. For now, the prefix is usually the benchmark
  suite from which we took most of the patches for that bug.

This policy is an ongoing goal rather than something already completed for
every benchmark in the tree.

# References

- [SCTBench](https://github.com/mc-imperial/sctbench.git): Paul Thomson,
  Alastair F. Donaldson, and Adam Betts, [Concurrency Testing Using Controlled
  Schedulers: An Empirical
  Study](https://dblp.org/rec/journals/topc/ThomsonDB16), ACM Transactions on
  Parallel Computing 2(4), 2016.
- [ConVul](https://github.com/mryancai/ConVul)
- [libvsync](https://github.com/open-s4c/libvsync)
- [PERIOD](https://github.com/wcventure/PERIOD): Cheng Wen, Mengda He, Bohao Wu,
  Zhiwu Xu, and Shengchao Qin, [Controlled Concurrency Testing via Periodical
  Scheduling](https://doi.org/10.1145/3510003.3510178), ICSE 2022.
- [concurrency-bugs](https://github.com/jieyu/concurrency-bugs): Shan Lu, Soyeon
  Park, Eunsoo Seo, and Yuanyuan Zhou, [Learning from Mistakes: A Comprehensive
  Study on Real World Concurrency Bug
  Characteristics](https://doi.org/10.1145/1346281.1346323), ASPLOS 2008.

Each directory should contain a `LICENSE` file when needed and, when upstream
provenance needs to be recorded, a `REFERENCE` file describing the relevant
software version, source, and URL.

# ThreadSanitizer

At the moment, the main build-time option exposed by Benchamel is whether
benchmarks are built with ThreadSanitizer.

To enable it, configure CMake with:

```sh
cmake -S . -B build -DBENCHAMEL_BUILD_TSAN=ON
```

This adds `-fsanitize=thread -g -fno-omit-frame-pointer` to benchmark targets.
When a benchmark builds downloaded dependencies, the same sanitizer flags
should also be propagated into those dependencies so the benchmark and its
dependencies are built consistently. For example, `memcached-127` propagates
them into its external `libevent` build.

# License

Most subdirectories contain the license file under which the contents of the
directory are distributed. Other software components in this repository are
released under the MIT license (see [LICENSE](LICENSE)).
