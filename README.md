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

Each directory contains a LICENSE and a VERSION file indicating the respective
license, software version and URL.

# License

Most subdirectories contain the license file under which the contents of the
directory are distributed. Other software components in this repository are
released under the MIT license (see [LICENSE](LICENSE)).
