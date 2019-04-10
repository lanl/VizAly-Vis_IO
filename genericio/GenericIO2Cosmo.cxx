/*
 *                    Copyright (C) 2015, UChicago Argonne, LLC
 *                               All Rights Reserved
 * 
 *                               Generic IO (ANL-15-066)
 *                     Hal Finkel, Argonne National Laboratory
 * 
 *                              OPEN SOURCE LICENSE
 * 
 * Under the terms of Contract No. DE-AC02-06CH11357 with UChicago Argonne,
 * LLC, the U.S. Government retains certain rights in this software.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 * 
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 * 
 *   3. Neither the names of UChicago Argonne, LLC or the Department of Energy
 *      nor the names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 * 
 * *****************************************************************************
 * 
 *                                  DISCLAIMER
 * THE SOFTWARE IS SUPPLIED “AS IS” WITHOUT WARRANTY OF ANY KIND.  NEITHER THE
 * UNTED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR
 * UCHICAGO ARGONNE, LLC, NOR ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY,
 * EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE
 * ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, DATA, APPARATUS,
 * PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE
 * PRIVATELY OWNED RIGHTS.
 * 
 * *****************************************************************************
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>
#include <cassert>

#include "GenericIO.h"

#define POSVEL_T float
#define ID_T int64_t
#define MASK_T uint16_t

using namespace std;
using namespace gio;

// This code is based on restart2cosmo.cxx
int main(int argc, char *argv[]) {
#ifndef GENERICIO_NO_MPI
  MPI_Init(&argc, &argv);
#endif

  int commRank, commRanks;
#ifndef GENERICIO_NO_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
  MPI_Comm_size(MPI_COMM_WORLD, &commRanks);
#else
  commRank = 0;
  commRanks = 1;
#endif

  if(argc != 3 + commRanks) {
    fprintf(stderr,"USAGE: %s <mpiioName> <cosmoName> <rank0> <rank1> ...\n",argv[0]);
    exit(-1);
  }

  char *mpiioName = argv[1];
  char *cosmoName = argv[2];
  int rank = atoi(argv[3 + commRank]);

  vector<POSVEL_T> xx, yy, zz, vx, vy, vz, phi;
  vector<ID_T> id;
  vector<MASK_T> mask;

  assert(sizeof(ID_T) == 8);

  size_t Np = 0;
  { // create GIO

  unsigned Method = GenericIO::FileIOPOSIX;
#ifndef GENERICIO_NO_MPI
  const char *EnvStr = getenv("GENERICIO_USE_MPIIO");
  if (EnvStr && string(EnvStr) == "1")
    Method = GenericIO::FileIOMPI;
#endif

  GenericIO GIO(
#ifndef GENERICIO_NO_MPI
    MPI_COMM_WORLD,
#endif
    mpiioName, Method);
  GIO.openAndReadHeader(GenericIO::MismatchAllowed);

  int NR = GIO.readNRanks();
  if (rank >= NR) {
    fprintf(stderr,"rank %d is invalid: file has data from %d ranks\n", rank, NR);
    fflush(stderr);
    exit(-1);
  }

#ifndef GENERICIO_NO_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  Np = GIO.readNumElems(rank);

  xx.resize(Np + GIO.requestedExtraSpace()/sizeof(POSVEL_T));
  yy.resize(Np + GIO.requestedExtraSpace()/sizeof(POSVEL_T));
  zz.resize(Np + GIO.requestedExtraSpace()/sizeof(POSVEL_T));
  vx.resize(Np + GIO.requestedExtraSpace()/sizeof(POSVEL_T));
  vy.resize(Np + GIO.requestedExtraSpace()/sizeof(POSVEL_T));
  vz.resize(Np + GIO.requestedExtraSpace()/sizeof(POSVEL_T));
  phi.resize(Np + GIO.requestedExtraSpace()/sizeof(POSVEL_T));
  id.resize(Np + GIO.requestedExtraSpace()/sizeof(ID_T));
  mask.resize(Np + GIO.requestedExtraSpace()/sizeof(MASK_T));

  GIO.addVariable("x", xx, true);
  GIO.addVariable("y", yy, true);
  GIO.addVariable("z", zz, true);
  GIO.addVariable("vx", vx, true);
  GIO.addVariable("vy", vy, true);
  GIO.addVariable("vz", vz, true);
  GIO.addVariable("phi", phi, true);
  GIO.addVariable("id", id, true);
  GIO.addVariable("mask", mask, true);

  GIO.readData(rank);
  } // destroy GIO

  FILE *cosmoFile =
    fopen((string(cosmoName) + "." + string(argv[3 + commRank])).c_str(), "wb");
  for(size_t i=0; i<Np; i++) {
    fwrite(&xx[i], sizeof(POSVEL_T), 1, cosmoFile);
    fwrite(&vx[i], sizeof(POSVEL_T), 1, cosmoFile);
    fwrite(&yy[i], sizeof(POSVEL_T), 1, cosmoFile);
    fwrite(&vy[i], sizeof(POSVEL_T), 1, cosmoFile);
    fwrite(&zz[i], sizeof(POSVEL_T), 1, cosmoFile);
    fwrite(&vz[i], sizeof(POSVEL_T), 1, cosmoFile);
    fwrite(&phi[i], sizeof(POSVEL_T), 1, cosmoFile);
    fwrite(&id[i], sizeof(ID_T), 1, cosmoFile);
  }
  fclose(cosmoFile);

#ifndef GENERICIO_NO_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
#endif

  return 0;
}

