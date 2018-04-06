#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

#include "GenericIO.h"

using namespace gio;

int main(int argc, char* argv[])
{
	std::string filename(argv[1]);
	//std::stringstream log;

	//
	// MPI Init
	int myRank, numRanks;
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm Comm = MPI_COMM_WORLD;


	srand (time(NULL) + myRank);
	
	{
		int dims[3]= {2, 2, 2};
		int periods[3] = { 0, 0, 0 };
		int physOrigin[3] = {0, 0, 0};
		int physScale[3] = {256, 256, 256};
		int numParticles = 100;


		MPI_Cart_create(Comm, 3, dims, periods, 0, &Comm);
		
		unsigned method = GenericIO::FileIOMPI;
		//unsigned method = GenericIO::FileIOPOSIX;

		filename.append(".oct");
		GenericIO newGIO(Comm, filename);//, method);
		newGIO.setNumElems(numParticles);

        for (int d = 0; d < 3; ++d)
        {
            newGIO.setPhysOrigin(physOrigin[d], d);
            newGIO.setPhysScale(physScale[d], d);
        }


		newGIO.addOctreeHeader((uint64_t)0, (uint64_t)1, (uint64_t)8);
		uint64_t extents[6]={0,256, 0,256, 0,256};
		float simExtents[6]={0,256, 0,256, 0,256};
		for (int i=0; i<8; i++)
		{
			newGIO.addOctreeRow(i,extents, 1000, 0, i);
		}
		
		
		//
		// Variables
		std::vector<float> xx, yy, zz, vx, vy, vz, phi;
		std::vector<uint16_t> mask;
		std::vector<int64_t> id;

		xx.resize(numParticles   + newGIO.requestedExtraSpace() / sizeof(float));
		yy.resize(numParticles   + newGIO.requestedExtraSpace() / sizeof(float));
		zz.resize(numParticles   + newGIO.requestedExtraSpace() / sizeof(float));
		vx.resize(numParticles   + newGIO.requestedExtraSpace() / sizeof(float));
		vy.resize(numParticles   + newGIO.requestedExtraSpace() / sizeof(float));
		vz.resize(numParticles   + newGIO.requestedExtraSpace() / sizeof(float));
		phi.resize(numParticles  + newGIO.requestedExtraSpace() / sizeof(float));
		id.resize(numParticles   + newGIO.requestedExtraSpace() / sizeof(int64_t));
		mask.resize(numParticles + newGIO.requestedExtraSpace() / sizeof(uint16_t));
		

		int offsetX, offsetY, offsetZ;
		int _8RankOffset = 128;
		if (myRank == 0) //
		{
			offsetX = 0;	offsetY = 0;	offsetZ = 0;
		}
		else if (myRank == 1)
		{
			offsetX = 0;	offsetY = 0;	offsetZ = _8RankOffset;
		}
		else if (myRank == 2)
		{
			offsetX = 0;	offsetY = _8RankOffset;	offsetZ = 0;
		}
		else if (myRank == 3)
		{
			offsetX = 0;	offsetY = _8RankOffset;	offsetZ = _8RankOffset;
		}
		else if (myRank == 4) //
		{
			offsetX = _8RankOffset;	offsetY = 0;	offsetZ = 0;
		}
		else if (myRank == 5)
		{
			offsetX = _8RankOffset;	offsetY = 0;	offsetZ = _8RankOffset;
		}
		else if (myRank == 6)
		{
			offsetX = _8RankOffset;	offsetY = _8RankOffset;	offsetZ = 0;
		}
		else if (myRank == 7)
		{
			offsetX = _8RankOffset;	offsetY = _8RankOffset;	offsetZ = _8RankOffset;
		}

		MPI_Barrier(MPI_COMM_WORLD);
		//log << offsetX << ", " << offsetY << ", " << offsetZ << std::endl;


		double randomNum;
		for (uint64_t i=0; i<numParticles; i++)
		{
			randomNum = ((rand() % 12800)/100.0) + offsetX;	//log << randomNum << ", ";
			xx[i] = randomNum;	

			randomNum = ((rand() % 12800)/100.0) + offsetY;	//log << randomNum << ", ";
			yy[i] = randomNum;

			randomNum = ((rand() % 12800)/100.0) + offsetZ;	//log << randomNum << "\n";
			zz[i] = randomNum;

			vx[i] = (double) (rand() % 1000 / 1000.0);
			vy[i] = (double) (rand() % 1000 / 1000.0);
			vz[i] = (double) (rand() % 1000 / 1000.0);
			phi[i] = (double) (rand() % 1000 / 100.0);
			mask[i] = (uint16_t)myRank;
			id[i] = myRank*numParticles + i;
		}


		unsigned CoordFlagsX = GenericIO::VarIsPhysCoordX;
        unsigned CoordFlagsY = GenericIO::VarIsPhysCoordY;
        unsigned CoordFlagsZ = GenericIO::VarIsPhysCoordZ;



		newGIO.addVariable("x", xx, CoordFlagsX | GenericIO::VarHasExtraSpace);
        newGIO.addVariable("y", yy, CoordFlagsY | GenericIO::VarHasExtraSpace);
		newGIO.addVariable("z", zz, CoordFlagsZ | GenericIO::VarHasExtraSpace);
		newGIO.addVariable("vx", vx, GenericIO::VarHasExtraSpace);
        newGIO.addVariable("vy", vy, GenericIO::VarHasExtraSpace);
        newGIO.addVariable("vz", vz, GenericIO::VarHasExtraSpace);
        newGIO.addVariable("phi", phi, GenericIO::VarHasExtraSpace);
		newGIO.addVariable("id", id, GenericIO::VarHasExtraSpace);
		newGIO.addVariable("mask", mask, GenericIO::VarHasExtraSpace);


		MPI_Barrier(MPI_COMM_WORLD);

		//
		// Octree settings

		// numLevels = 2, leaves overall=64
		// 		in current test, each rank has 8 leaves
		//

		float rankExtents[6];
		int numLevels = 2;
		int displayRank = 0;

		rankExtents[0] = offsetX;	rankExtents[1] = offsetX + _8RankOffset;
		rankExtents[2] = offsetY;	rankExtents[3] = offsetY + _8RankOffset;
		rankExtents[4] = offsetZ;	rankExtents[5] = offsetZ + _8RankOffset;
		Octree gioOctree(numLevels, simExtents);
		gioOctree.buildOctree();

		int numOctreeLeaves = gioOctree.getNumNodes();
		int numLeavesPerRank = numOctreeLeaves/numRanks;
		int numleavesForMyRank = numLeavesPerRank;

		if (myRank == displayRank)
		{
			std::cout <<myRank << " ~ " << rankExtents[0] << "-" << rankExtents[1] << ", "
										<< rankExtents[2] << "-" << rankExtents[3] << ", "
										<< rankExtents[4] << "-" << rankExtents[5] << std::endl;

			std::cout << myRank << " ~ " << numOctreeLeaves << ", " << numLeavesPerRank << std::endl;
		}

		//
        // Storing number of partitions per rank
        std::vector< int > myLeaves;
        float myOctreeExtents[6];
        myOctreeExtents[0]=myOctreeExtents[2]=myOctreeExtents[4]=std::numeric_limits<float>::max();
        myOctreeExtents[1]=myOctreeExtents[3]=myOctreeExtents[5]=std::numeric_limits<float>::min();

        for (int i=0; i<numLeavesPerRank; i++)
        {
            int leafID = myRank*numleavesForMyRank + i;
            myLeaves.push_back(leafID);

            float _extents[6];
            gioOctree.getLeafExtents(leafID,_extents);
            myOctreeExtents[0]=std::min(_extents[0], myOctreeExtents[0]);
            myOctreeExtents[2]=std::min(_extents[2], myOctreeExtents[2]);
            myOctreeExtents[4]=std::min(_extents[4], myOctreeExtents[4]);

            myOctreeExtents[1]=std::max(_extents[1], myOctreeExtents[1]);
            myOctreeExtents[3]=std::max(_extents[3], myOctreeExtents[3]);
            myOctreeExtents[5]=std::max(_extents[5], myOctreeExtents[5]);

            //debugLog.addLog( std::to_string(i) + ": " + std::to_string(leafID) + ", " );

			// std::cout <<myRank << " ~ " << leafID << " | " 
			// 			<< myOctreeExtents[0] << "-" << myOctreeExtents[1] << ", "
			// 			<< myOctreeExtents[2] << "-" << myOctreeExtents[3] << ", "
			// 			<< myOctreeExtents[4] << "-" << myOctreeExtents[5] << std::endl;
			if (myRank == displayRank)
				std::cout <<myRank << " ~ " << leafID << " | " 
							<< _extents[0] << "-" << _extents[1] << ", "
							<< _extents[2] << "-" << _extents[3] << ", "
							<< _extents[4] << "-" << _extents[5] << std::endl;
        }

        newGIO.write();



		MPI_Barrier(MPI_COMM_WORLD);
	}

	
	//std::ofstream outputFile( ("dataGEn_" + std::to_string(myRank) + "_.log").c_str(), std::ios::out);
	//outputFile << log.str();
	//outputFile.close();

	
	MPI_Finalize();

	return 0;
}

// mpirun -np 8 ./dataGen outputFile