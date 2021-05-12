# Parallel PageRank
Implements the PageRank algorithm by Larry Page in parallel.

## Usage
This script is intended to be run on SHARCNET computers. The following are some simple bash commands to view the job progress and results.

### Running Interactively
> Note: Since this script is written in Python, I recommend running interactively.
```bash
module load python mpi4py
salloc --time=0:10:00 --tasks=8 --mem-per-cpu=1024M
chmod a+x pagerank.py # Give execution permission to the cluster
srun pagerank.py
```

### Opening Output Slurm
```bash
more slurm-[job-id]
```