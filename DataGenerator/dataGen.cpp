#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

#include "../genericio/GenericIO.h"

using namespace gio;

int main(int argc, char* argv[])
{
	std::string filename(argv[1]);
	srand (time(NULL));

	//
	// MPI Init
	int myRank, numRanks;
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm Comm = MPI_COMM_WORLD;

	{
		int dims[3]= {2, 2, 2};
		int periods[3] = { 0, 0, 0 };
		int physOrigin[3] = {0, 0, 0};
		int physScale[3] = {256, 256, 256};
		int numParticles = 1000;


		MPI_Cart_create(Comm, 3, dims, periods, 0, &Comm);

		GenericIO newGIO(Comm, filename);
		newGIO.setNumElems(numParticles);

        for (int d = 0; d < 3; ++d)
        {
            newGIO.setPhysOrigin(physOrigin[d], d);
            newGIO.setPhysScale(physScale[d], d);
        }

		//
		// Variables
		double *xx, *yy, *zz, *vx, *vy, *vz, *phi;
		uint16_t* mask;

		xx = new double[numParticles];
		yy = new double[numParticles];
		zz = new double[numParticles];
		vx = new double[numParticles];
		vy = new double[numParticles];
		vz = new double[numParticles];
		phi = new double[numParticles];
		mask = new uint16_t[numParticles];

		int offsetX, offsetY, offsetZ;
		// if (myRank == 0) //
		// {
		// 	offsetX = 0;
		// 	offsetY = 0;
		// 	offsetZ = 0;
		// }
		// else if (myRank == 1)
		// {
		// 	offsetX = 0;
		// 	offsetY = 0;
		// 	offsetZ = 128;
		// }
		// else if (myRank == 2)
		// {
		// 	offsetX = 0;
		// 	offsetY = 128;
		// 	offsetZ = 0;
		// }
		// else if (myRank == 3)
		// {
		// 	offsetX = 0;
		// 	offsetY = 128;
		// 	offsetZ = 128;
		// }
		// else if (myRank == 4) //
		// {
		// 	offsetX = 128;
		// 	offsetY = 0;
		// 	offsetZ = 0;
		// }
		// else if (myRank == 5)
		// {
		// 	offsetX = 128;
		// 	offsetY = 0;
		// 	offsetZ = 128;
		// }
		// else if (myRank == 6)
		// {
		// 	offsetX = 128;
		// 	offsetY = 128;
		// 	offsetZ = 0;
		// }
		// else if (myRank == 7)
		// {
		// 	offsetX = 128;
		// 	offsetY = 128;
		// 	offsetZ = 128;
		// }



		if (myRank == 0) //
		{
			offsetX = 64;
			offsetY = 64;
			offsetZ = 64;
		}
		else if (myRank == 1)
		{
			offsetX = 64;
			offsetY = 64;
			offsetZ = 192;
		}
		else if (myRank == 2)
		{
			offsetX = 0;
			offsetY = 0;
			offsetZ = 0;
		}
		else if (myRank == 3)
		{
			offsetX = 0;
			offsetY = 0;
			offsetZ = 0;
		}
		else if (myRank == 4) //
		{
			offsetX = 0;
			offsetY = 0;
			offsetZ = 0;
		}
		else if (myRank == 5)
		{
			offsetX = 0;
			offsetY = 0;
			offsetZ = 0;
		}
		else if (myRank == 6)
		{
			offsetX = 0;
			offsetY = 0;
			offsetZ = 0;
		}
		else if (myRank == 7)
		{
			offsetX = 0;
			offsetY = 0;
			offsetZ = 0;
		}


		for (uint64_t i=0; i<numParticles; i++)
		{

			// xx[i] = (double) (offsetX + rand() % 128);
			// yy[i] = (double) (offsetY + rand() % 128);
			// zz[i] = (double) (offsetZ + rand() % 128);
			// vx[i] = (double) (rand() % 1000 / 1000.0);
			// vy[i] = (double) (rand() % 1000 / 1000.0);
			// vz[i] = (double) (rand() % 1000 / 1000.0);
			// phi[i] = (double) (rand() % 1000 / 100.0);
			// mask[i] = (uint16_t)myRank;


			xx[i] = (double) (offsetX);
			yy[i] = (double) (offsetY);
			zz[i] = (double) (offsetZ);
			vx[i] = (double) (rand() % 1000 / 1000.0);
			vy[i] = (double) (rand() % 1000 / 1000.0);
			vz[i] = (double) (rand() % 1000 / 1000.0);
			phi[i] = (double) (rand() % 1000 / 100.0);
			mask[i] = (uint16_t)myRank;


		}

		int CoordFlagsX = GenericIO::VarIsPhysCoordX;
        int CoordFlagsY = GenericIO::VarIsPhysCoordY;
        int CoordFlagsZ = GenericIO::VarIsPhysCoordZ;

		//GIO.addVariable("x", xx, CoordFlagsX | GenericIO::VarHasExtraSpace);
		newGIO.addVariable("x", xx, CoordFlagsX);
        newGIO.addVariable("y", yy, CoordFlagsY);
		newGIO.addVariable("z", zz, CoordFlagsZ);
		newGIO.addVariable("vx", vx);
        newGIO.addVariable("vy", vy);
        newGIO.addVariable("vz", vz);
        newGIO.addVariable("phi", phi);
		newGIO.addVariable("mask", mask);


		

        newGIO.write();
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}

// mpirun -np 8 ./dataGen