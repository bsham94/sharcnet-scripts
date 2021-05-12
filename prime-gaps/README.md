# Prime Gaps
Calculates the largest gap between a pair of consecutive prime numbers.

## Usage
This C file is intended to be run on SHARCNET computers. The following are some simple bash commands to view the job progress and results.

### Compiling with MPI
```bash
mpicc primes.c -o primes -lmpi
```

### Queuing the Job
> Note: Make sure to modify `submit_primes.sh` before queuing. 

```bash
sbatch submit_primes.sh
```

### Viewing Job Queue
> Note: This will show all jobs currently running under your login name. Your login name may be different than your account name on SHARCNET.

```bash
squeue -u <LOGIN NAME>
```

### Opening Output Slurm
```bash
more slurm-[job-id]
```