#! /bin/bash
#SBATCH --account=def-ikotsire  # cp431g4 throws error
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=128M       # memory; default unit is megabytes
#SBATCH --time=0-00:05          # time (DD-HH:MM)
srun bprimes              # mpirun or mpiexec also work