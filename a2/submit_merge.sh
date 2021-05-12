#! /bin/bash
#SBATCH --account=<ACCOUNT NAME HERE>
#SBATCH --ntasks=8                  # number of processors
#SBATCH --mem-per-cpu=1024M         # memory; default unit is megabytes
#SBATCH --time=0-05:00              # time (DD-HH:MM)

module load python/3.6 mpi4py
virtualenv --no-download $SLURM_TMPDIR/env
source $SLURM_TMPDIR/env/bin/activate
pip install --no-index --upgrade pip

pip install --no-index -r requirements.txt
chmod a+x merge.py
srun ./merge.py