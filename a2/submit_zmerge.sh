#! /bin/bash
#SBATCH --account=def-ikotsire  # cp431g4 throws error
#SBATCH --ntasks=32             # number of processors + 1
#SBATCH --mem-per-cpu=1024M     # memory; default unit is megabytes
#SBATCH --time=0-0:30           # time (DD-HH:MM)
srun zmerge                     # mpirun or mpiexec also work