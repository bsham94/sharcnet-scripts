# Parallel Merge
Uses parallel MPI techniques to merges 2 large sorted arrays.

## Usage
This script is intended to be run on SHARCNET computers. The following are some simple bash commands to view the job progress and results.

### Running Interactively
> Note: Since this script is written in Python, I recommend running interactively.
```bash
module load python mpi4py
salloc --time=0:10:00 --tasks=8 --mem-per-cpu=1024M
chmod a+x merge.py # Give execution permission to the cluster
srun merge.py
```

### Opening Output Slurm
```bash
more slurm-[job-id]
```