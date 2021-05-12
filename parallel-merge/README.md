## QUEUE
`sbatch submit_merge.sh`

## VIEW
`squeue -u cp431g4`

## OUTPUT
`more slurm-[job-id]`

## RUNNING INTERACTIVELY
 - `module load python mpi4py`
 - `salloc --time=0:10:00 --tasks=8 --mem-per-cpu=1024M`
 - `chmod a+x merge.py`
 - `srun merge.py`