#!/bin/bash
#SBATCH --partition=galton
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=16
date
mkdir -p /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-401/analysis/spectrum
cd /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-401/analysis/spectrum
source /projects/exasky/HACC/trunk/env/bashrc.darwin.cpu
mpirun /projects/exasky/HACC/trunk/darwin.cpu/mpi/bin/hacc_pk_gio_auto_rsd /projects/exasky/pascal-projects/VizAly-Foresight/inputs/hacc/halo_inputs/indat.params -n /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-401/reduction/cbench/decompressed_files/sz_PosNoComp_vel_.001__m000.full.mpicosmo.401 sz_PosNoComp_vel_.001 499
date
