# SCTBench

A set of C/C++ pthread benchmarks and tools for evaluating concurrency testing techniques.

You can download our original VirtualBox artifact to reproduce our experiments [here](https://sites.google.com/site/sctbenchmarks/).
This git repository is supposed to be more convenient and includes more recent changes,
such as our implementation of the PCT algorithm.
Usability of the tools and scripts could still be greatly improved.
Nevertheless, it should be possible to reproduce our experimental evaluation.

Recall that the data race detection phase is nondeterministic
and different compiler versions may also lead to different binaries.
Thus, our study is not deterministic.
However, we provide a link to our log files so
additional analysis of our results is possible and
the graphs and tables can be reproduced deterministically. 
