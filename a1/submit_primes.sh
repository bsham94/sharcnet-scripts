#! /bin/bash
#SBATCH --account=def-ikotsire  # cp431g4 throws error
#SBATCH --nodes=8               # number of processors + 1; node 0 will do nothing
#SBATCH --ntasks-per-node=1     
#SBATCH --mem-per-cpu=1024M     # memory; default unit is megabytes
#SBATCH --time=0-00:30          # time (DD-HH:MM)
srun primes                     # mpirun or mpiexec also work