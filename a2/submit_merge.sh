#! /bin/bash
#SBATCH --account=def-ikotsire  # cp431g4 throws error
#SBATCH --ntasks=19             # number of processors + 1
#SBATCH --mem-per-cpu=1024M     # memory; default unit is megabytes
#SBATCH --time=0-0:30           # time (DD-HH:MM)

module load python/3.6 mpi4py
virtualenv --no-download $SLURM_TMPDIR/env
source $SLURM_TMPDIR/env/bin/activate
pip install --no-index --upgrade pip

pip install --no-index -r requirements.txt
chmod a+x merge.py
srun ./merge.py