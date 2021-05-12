<img width="210" src="sharcnet.png">

# SHARCNET Scripts
A collection of scripts built for use on SHARCNET supercomputers.

## About
There are 3 main scripts included in this repository. They are designed to run large jobs, and not intended to run synchronously.
### Prime Gaps `.\prime-gaps`
 * Calculates the largest gap between a pair of consecutive prime numbers.
 * Written in C, using MPI.h and GMP.h for Linux.
### Parallel Merge `.\parallel-merge`
 * Merges 2 large sorted arrays.
 * Written in Python, using MPI4PY.
### PageRank `.\pagerank`
 * Implements the PageRank algorithm by Larry Page in parallel.
 * Written in Python, using MPI4PY.