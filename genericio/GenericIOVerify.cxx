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
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <stdint.h>

#include "GenericIO.h"

using namespace gio;
using namespace std;

int main(int argc, char *argv[]) {
#ifndef GENERICIO_NO_MPI
  MPI_Init(&argc, &argv);
#endif

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " [-v] <mpiioName1> <mpiioName2> ..." << endl;
    exit(-1);
  }

  int arg = 1;
  bool Verbose = false;
  if (string(argv[arg]) == "-v") {
    Verbose = true;
    ++arg;
  }

  int Rank, NRanks;
#ifndef GENERICIO_NO_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
  MPI_Comm_size(MPI_COMM_WORLD, &NRanks);
#else
  Rank = 0;
  NRanks = 1;
#endif

  bool AllOkay = true;

  for (; arg < argc; ++arg) {
    string FileName(argv[arg]);

    try {
      if (Verbose) cout << "verifying: " << FileName << endl;

      unsigned Method = GenericIO::FileIOPOSIX;
#ifndef GENERICIO_NO_MPI
      const char *EnvStr = getenv("GENERICIO_USE_MPIIO");
      if (EnvStr && string(EnvStr) == "1")
        Method = GenericIO::FileIOMPI;
#endif

#ifndef GENERICIO_NO_MPI
      GenericIO GIO(MPI_COMM_WORLD, FileName, Method);
#else
      GenericIO GIO(FileName, Method);
#endif

#ifndef GENERICIO_NO_MPI
      bool MustMatch = true;
#else
      bool MustMatch = false;
#endif
      GIO.openAndReadHeader(MustMatch ? GenericIO::MismatchRedistribute :
                                        GenericIO::MismatchDisallowed);
      if (Verbose) cout << "\theader: okay" << endl;


      vector<GenericIO::VariableInfo> VI;
      GIO.getVariableInfo(VI);

#ifndef GENERICIO_NO_MPI
      size_t MaxNElem = GIO.readNumElems();
#else
      size_t MaxNElem = 0;
      int NR = GIO.readNRanks();
      for (int i = 0; i < NR; ++i) {
        size_t NElem = GIO.readNumElems(i);
        MaxNElem = max(MaxNElem, NElem);
      }
#endif

      vector< vector<char> > Vars(VI.size());
      for (size_t i = 0; i < VI.size(); ++i) {
        Vars[i].resize(VI[i].Size*MaxNElem + GIO.requestedExtraSpace());
        GIO.addVariable(VI[i], &Vars[i][0], true);
      }

#ifndef GENERICIO_NO_MPI
      GIO.readData(-1, false);
      if (Verbose) cout << "\tdata from rank " << Rank << ": okay" << endl;
#else
      for (int i = 0; i < NR; ++i) {
        GIO.readData(i, false);
        if (Verbose) cout << "\tdata from rank " << i << ": okay" << endl;
      }
#endif

      if (Rank == 0) {
        cout << FileName << ": okay" << endl;
        cout.flush();
      }
    } catch (exception &e) {
      cerr << "ERROR: " << FileName << ": " << e.what() << endl;
      cerr.flush();
      AllOkay = false;
    }
  }

#ifndef GENERICIO_NO_MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
#endif

  return AllOkay ? 0 : 1;
}

