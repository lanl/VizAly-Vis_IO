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
  MPI_Init(&argc, &argv);

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <mpiioOld> <mpiioNew>" << endl;
    exit(-1);
  }

  GenericIO::setNaturalDefaultPartition();
  GenericIO::setDefaultShouldCompress(true);

  {
    int arg = 1;
    int Rank, NRanks;
    MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
    MPI_Comm_size(MPI_COMM_WORLD, &NRanks);

    string FileName(argv[arg++]);
    string NewFileName(argv[arg++]);

    unsigned Method = GenericIO::FileIOPOSIX;
    const char *EnvStr = getenv("GENERICIO_USE_MPIIO");
    if (EnvStr && string(EnvStr) == "1")
      Method = GenericIO::FileIOMPI;

    GenericIO GIO(MPI_COMM_WORLD, FileName, Method);
    GIO.openAndReadHeader(GenericIO::MismatchRedistribute);

    int NR = GIO.readNRanks();
    if (!Rank && NR != NRanks) {
      cout << "Redistributing data from " << NR << " ranks to " << NRanks <<
              " ranks; dropping rank topology information!\n";
    }

    vector<GenericIO::VariableInfo> VI;
    GIO.getVariableInfo(VI);

    size_t NElem = GIO.readNumElems();

    double PhysOrigin[3], PhysScale[3];
    GIO.readPhysOrigin(PhysOrigin);
    GIO.readPhysScale(PhysScale);

    vector< vector<char> > Vars(VI.size());
    for (size_t i = 0; i < VI.size(); ++i) {
      Vars[i].resize(VI[i].Size*NElem + GIO.requestedExtraSpace());
      GIO.addVariable(VI[i], &Vars[i][0], GenericIO::VarHasExtraSpace);
    }

    GIO.readData(-1, false);

    MPI_Comm Comm = MPI_COMM_WORLD;
    if (NR == NRanks) {
      int Periods[3] = { 0, 0, 0 };
      int Dims[3];
      GIO.readDims(Dims);
      MPI_Cart_create(Comm, 3, Dims, Periods, 0, &Comm);
    }

    GenericIO NewGIO(Comm, NewFileName);
    NewGIO.setNumElems(NElem);

    for (int d = 0; d < 3; ++d) {
      NewGIO.setPhysOrigin(PhysOrigin[d], d);
      NewGIO.setPhysScale(PhysScale[d], d);
    }

    for (size_t i = 0; i < VI.size(); ++i) {
      if (NR != NRanks) {
        // When dropping topology information, also drop the related column tags.
        VI[i].IsPhysCoordX = VI[i].IsPhysCoordY = VI[i].IsPhysCoordZ =
          VI[i].MaybePhysGhost = false;
      }

      NewGIO.addVariable(VI[i], &Vars[i][0], GenericIO::VarHasExtraSpace);
    }

    NewGIO.write();
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}

