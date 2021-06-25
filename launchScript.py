#!/bin/bash
#SBATCH --partition=galton
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=1
date

module load anaconda/Anaconda3
cd /projects/exasky/pascal-projects/VizAly-Vis_IO
mpiexec python testMPI.py
