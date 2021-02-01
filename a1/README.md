## BUILD
`mpicc primes.c -o primes -lmpi`

## RUN
`sbatch submit_primes.sh`

## VIEW
`squeue -u cp431g4`

## OUTPUT
`more slurm-[job-id]`