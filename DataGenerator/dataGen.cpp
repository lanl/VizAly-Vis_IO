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
		float simExtents[6]={0,256, 0,256, 0,256};
		int dims[3]= {2, 2, 2};
		int periods[3] = { 0, 0, 0 };
		int physOrigin[3] = {0, 0, 0};
		int physScale[3] = {256, 256, 256};
		size_t numParticles = 10000;


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
		if (myRank == 0)
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
		else if (myRank == 4)
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
		int displayRank = 1;
		int shuffle = 0;

		rankExtents[0] = offsetX;	rankExtents[1] = offsetX + _8RankOffset;
		rankExtents[2] = offsetY;	rankExtents[3] = offsetY + _8RankOffset;
		rankExtents[4] = offsetZ;	rankExtents[5] = offsetZ + _8RankOffset;
		Octree gioOctree(numLevels, simExtents);
		gioOctree.buildOctree();
		gioOctree.myRank = myRank;

		int numOctreeLeaves = gioOctree.getNumNodes();
		int numLeavesPerRank = numOctreeLeaves/numRanks;
		int numleavesForMyRank = numLeavesPerRank;

		if (myRank == displayRank)
		{
			std::cout <<myRank << " ~ " << rankExtents[0] << "-" << rankExtents[1] << ", "
										<< rankExtents[2] << "-" << rankExtents[3] << ", "
										<< rankExtents[4] << "-" << rankExtents[5] << std::endl;

			std::cout << myRank << " ~  # octree leaves: " << numOctreeLeaves << ", leaves/rank: " << numLeavesPerRank << std::endl;
		}



		//
        // Storing number of partitions per rank
        std::vector< int > myLeaves;


        int *leavesExtents = new int[numLeavesPerRank*6];

        for (int i=0; i<numLeavesPerRank; i++)
        {
            int leafID = myRank*numleavesForMyRank + i;
            myLeaves.push_back(leafID);

            float _extents[6];
            gioOctree.getLeafExtents(leafID,_extents);

            // Gathering extents of all leaves
            for (int j=0; j<6; j++)
            	leavesExtents[i*6 + j] = _extents[j];


			if (myRank == displayRank)
			{
				std::cout <<myRank << " ~ " << leafID << " | " 
							<< _extents[0] << "-" << _extents[1] << ", "
							<< _extents[2] << "-" << _extents[3] << ", "
							<< _extents[4] << "-" << _extents[5] << std::endl;
			}
        }


        



        // Rearrange the particles
        std::vector<int> partitionPosition;
        std::vector<int> partitionCount;
        float *_xx = &xx[0];
        float *_yy = &yy[0];
        float *_zz = &zz[0];

        partitionCount = gioOctree.findPartition(_xx,_yy,_zz, numParticles, numLeavesPerRank, leavesExtents, partitionPosition);


        

        if (myRank == displayRank)
			for (int i=0; i<numLeavesPerRank; i++)
				std::cout << myRank << " : " << partitionCount[i] << std::endl;

		float *_temp;
		_temp = &xx[0];	gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _temp, numParticles, false);
		_temp = &yy[0];	gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _temp, numParticles, false);
		_temp = &zz[0];	gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _temp, numParticles, false);
		_temp = &vx[0];	gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _temp, numParticles, false);
		_temp = &vy[0];	gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _temp, numParticles, false);
		_temp = &vz[0];	gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _temp, numParticles, false);
		_temp = &phi[0];gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _temp, numParticles, false);

		// Temp to get info
		uint16_t *_tempMask = &mask[0];
		gioOctree.fillArray(numLeavesPerRank, partitionCount, _tempMask, numParticles);
		//gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _tempMask, numParticles, false);

		int64_t *_tempId = &id[0];
		gioOctree.reorganizeArray(numLeavesPerRank, partitionCount, partitionPosition, _tempId, numParticles, false);		
		


		//MPI_Barrier(MPI_COMM_WORLD);
		int * nodesPerLeaves = new int[numOctreeLeaves];
        int *myLeavesCount;  myLeavesCount=&partitionCount[0];

        //for (int i=0; i<numLeavesPerRank; i++)
        //	std::cout << myRank << " ~ " << myLeavesCount[i] << std::endl;

        MPI_Allgather( myLeavesCount, numLeavesPerRank, MPI_INT,  nodesPerLeaves, numLeavesPerRank, MPI_INT,  MPI_COMM_WORLD); 

        MPI_Barrier(MPI_COMM_WORLD);
        // if (myRank == displayRank)
        // 	for (int i=0; i<numOctreeLeaves; i++)
        // 	std::cout << i << " ~ " << (uint64_t)nodesPerLeaves[i] << std::endl;


        newGIO.addOctreeHeader((uint64_t)shuffle, (uint64_t)numLevels, (uint64_t)numOctreeLeaves);
		uint64_t extents[6]={0,256, 0,256, 0,256};

		int leafCount = 0;
		for (int r=0; r<numRanks; r++)
		{
			uint64_t offsetInRank = 0;
			for (int l=0; l<numLeavesPerRank; l++)
        	{
        		int leafID = r*numleavesForMyRank + l;


            	float __extents[6];
            	gioOctree.getLeafExtents(leafID,__extents);

            	uint64_t _leafExtents[6];
            	for (int i=0; i<6; i++)
            		_leafExtents[i] = (uint64_t) round(__extents[i]);
            	//// 	newGIO.addOctreeRow(i,extents, 100, 0, i);
            	if (myRank == displayRank)
            		std::cout << nodesPerLeaves[leafCount] << std::endl;
            	newGIO.addOctreeRow(leafCount,_leafExtents, nodesPerLeaves[leafCount], offsetInRank, r);

            	leafCount++;
            	offsetInRank += nodesPerLeaves[leafCount];
        	}
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