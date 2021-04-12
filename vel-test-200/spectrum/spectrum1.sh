#!/bin/bash
#SBATCH --partition=galton
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=16
date
mkdir -p /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-203/analysis/spectrum
cd /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-203/analysis/spectrum
source /projects/exasky/HACC/trunk/env/bashrc.darwin.cpu
mpirun /projects/exasky/HACC/trunk/darwin.cpu/mpi/bin/hacc_pk_gio_auto_rsd /projects/exasky/pascal-projects/VizAly-Foresight/inputs/hacc/halo_inputs/indat.params -n /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-203/reduction/cbench/decompressed_files/BLOSC___m000.full.mpicosmo.203 BLOSC_ 499
date
