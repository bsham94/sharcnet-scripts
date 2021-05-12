#! /bin/bash
#SBATCH --account=<ACCOUNT NAME HERE>
#SBATCH --ntasks=8                  # number of processors
#SBATCH --mem-per-cpu=1024M         # memory; default unit is megabytes
#SBATCH --time=0-05:00              # time (DD-HH:MM)
srun primes                         # mpirun or mpiexec also work