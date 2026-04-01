# concurrency-bugs

This directory contains Benchamel's normalized copies of bugs that originate
from Jieyu Yu's `concurrency-bugs` collection and related follow-on uses of
those bugs.

For the concurrency-bugs family, this directory is the source of truth inside
Benchamel. When the same bug also appears in other suites in this repository,
such as PERIOD or SCTBench, we prefer to preserve and evolve the original
`concurrency-bugs/` copy here rather than treating the later copies as the
canonical source.

These bugs have been reused by other benchmark suites and papers, including:

- the original `concurrency-bugs` benchmark collection
- SCTBench's concurrency-bug set
- PERIOD's evaluation subjects

Each bug directory is gradually being normalized to follow the repository-wide
benchmark preparation rules from the top-level [README](../README.md), while
keeping provenance in patch headers, local `README.md` files, and `REFERENCE`
metadata where available.
