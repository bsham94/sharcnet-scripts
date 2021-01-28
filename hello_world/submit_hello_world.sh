#! /bin/bash
#SBATCH --account=cp431g4
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=3
#SBATCH --mem-per-cpu=32M       # memory; default unit is megabytes
#SBATCH --time=0-00:05          # time (DD-HH:MM)
srun ./hello-world              # mpirun or mpiexec also work