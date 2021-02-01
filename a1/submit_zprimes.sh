#! /bin/bash
#SBATCH --account=def-ikotsire  # cp431g4 throws error
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=1024M       # memory; default unit is megabytes
#SBATCH --time=0-00:05          # time (DD-HH:MM)
srun zprimes              # mpirun or mpiexec also work