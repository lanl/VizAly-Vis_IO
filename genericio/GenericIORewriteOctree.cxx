#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>
#include <cstdint> //or <stdint.h>
#include <limits>
#include <stdio.h>
#include <limits.h>

#include "GenericIO.h"
//#include "gioData.h"



int main(int argc, char *argv[])
{
	std::string filename = argv[1];		// input
	std::string _filename = argv[2];	// output
	int numOctreeLevels = atoi(argv[3]);


	std::vector<GioData> readInData;


	int myRank, numRanks;
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm comm_cart;


    if (myRank == 0)
		std::cout << "Input: " << filename << ", output: " << _filename << ", num levels: " << numOctreeLevels << std::endl;

	double physOrigin[3];
	double physScale[3];
	int mpiCartPartitions[3];

	size_t totalNumberOfElements = 0;

	//
	// Actually load the data
	int minmaxX[2] = {INT_MAX, INT_MIN};
	int minmaxY[2] = {INT_MAX, INT_MIN};
	int minmaxZ[2] = {INT_MAX, INT_MIN};


	Timer clock;
	std::stringstream log;

	clock.start();

	{
		gio::GenericIO *gioReader;


		// Init GenericIO reader + open file
		gioReader = new gio::GenericIO(comm, filename);

		// Open file
		gioReader->openAndReadHeader(gio::GenericIO::MismatchRedistribute);
		int numDataRanks = gioReader->readNRanks();


		gioReader->readPhysOrigin(physOrigin);
	    gioReader->readPhysScale(physScale);

		size_t numElements = 0;
		for (int i = 0; i < numDataRanks; i++)
			numElements += gioReader->readNumElems(i);


		if (numRanks > numDataRanks)
		{
			std::cout << "Num data ranks: " << numDataRanks << "Use <= MPI ranks than data ranks" << std::endl;
			return -1;
		}


		// Read in the scalars information
		std::vector<gio::GenericIO::VariableInfo> VI;
		gioReader->getVariableInfo(VI);
		int numVars = static_cast<int>(VI.size());

		
		readInData.resize(numVars);
		for (int i = 0; i < numVars; i++)
			readInData[i].init(i, VI[i].Name, static_cast<int>(VI[i].Size), VI[i].IsFloat, VI[i].IsSigned, VI[i].IsPhysCoordX, VI[i].IsPhysCoordY, VI[i].IsPhysCoordZ);



		//
		// Split ranks among data
		int numDataRanksPerMPIRank = numDataRanks / numRanks;
		int loadRange[2];
		loadRange[0] = myRank * numDataRanksPerMPIRank;
		loadRange[1] = (myRank + 1) * numDataRanksPerMPIRank;
		if (myRank == numRanks - 1)
			loadRange[1] = numDataRanks;

		int splitDims[3];
		gioReader->readDims(splitDims);
		log << "splitDims: " << splitDims[0] << "," << splitDims[1] << "," << splitDims[2] << std::endl;



		//
		// Determine memory size and allocate memory
		for (int i = loadRange[0]; i < loadRange[1]; i++)
			totalNumberOfElements += gioReader->readNumElems(i);

		for (int i = 0; i < numVars; i++)
		{
			readInData[i].setNumElements(totalNumberOfElements);
			readInData[i].allocateMem(1);
		}

		log << "totalNumberOfElements to read in: " << totalNumberOfElements << std::endl;
		if (myRank == 0)
			std::cout << "totalNumberOfElements to read in: " << totalNumberOfElements << std::endl;


		
		//
		// Actually load the data
		size_t offset = 0;
		for (int i = loadRange[0]; i < loadRange[1]; i++) // for each rank
		{
			size_t Np = gioReader->readNumElems(i);

			int coords[3];
			gioReader->readCoords(coords, i);
			log << "Coord indices: " << coords[0] << ", " << coords[1] << ", " << coords[2] << " | ";

			log << "coordinates: (" << (float)coords[0] / splitDims[0] * physScale[0] + physOrigin[0] << ", "
	                          << (float)coords[1] / splitDims[1] * physScale[1] + physOrigin[1] << ", "
	                          << (float)coords[2] / splitDims[2] * physScale[2] + physOrigin[2] << ") -> ("
	                          << (float)(coords[0] + 1) / splitDims[0] * physScale[0] + physOrigin[0] << ", "
	                          << (float)(coords[1] + 1) / splitDims[1] * physScale[1] + physOrigin[1] << ", "
	                          << (float)(coords[2] + 1) / splitDims[2] * physScale[2] + physOrigin[2] << ")" << std::endl;


	        minmaxX[0] = std::min( (int) ((float)coords[0] / splitDims[0] * physScale[0] + physOrigin[0]), minmaxX[0] );
	        minmaxY[0] = std::min( (int) ((float)coords[1] / splitDims[1] * physScale[1] + physOrigin[1]), minmaxY[0] );
	        minmaxZ[0] = std::min( (int) ((float)coords[2] / splitDims[2] * physScale[2] + physOrigin[2]), minmaxZ[0] );

	        minmaxX[1] = std::max( (int) ((float)(coords[0] + 1) / splitDims[0] * physScale[0] + physOrigin[0]), minmaxX[1] );
	        minmaxY[1] = std::max( (int) ((float)(coords[1] + 1) / splitDims[1] * physScale[1] + physOrigin[1]), minmaxY[1] );
	        minmaxZ[1] = std::max( (int) ((float)(coords[2] + 1) / splitDims[2] * physScale[2] + physOrigin[2]), minmaxZ[1] );
	        
			
			for (int j=0; j<numVars; j++)
			{
				if (readInData[j].dataType == "float")
					gioReader->addVariable( (readInData[j].name).c_str(), &((float*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "double")
					gioReader->addVariable( (readInData[j].name).c_str(), &((double*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "int8_t")
					gioReader->addVariable( (readInData[j].name).c_str(),  &((int8_t*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "int16_t")
					gioReader->addVariable( (readInData[j].name).c_str(), &((int16_t*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "int32_t")
					gioReader->addVariable( (readInData[j].name).c_str(), &((int32_t*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "int64_t")
					gioReader->addVariable( (readInData[j].name).c_str(), &((int64_t*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "uint8_t")
					gioReader->addVariable( (readInData[j].name).c_str(), &((uint8_t*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "uint16_t")
					gioReader->addVariable( (readInData[j].name).c_str(), &((uint16_t*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "uint32_t")
					gioReader->addVariable( (readInData[j].name).c_str(), &((uint32_t*)readInData[j].data)[offset], true);
				else if (readInData[j].dataType == "uint64_t")
					gioReader->addVariable( (readInData[j].name).c_str(), &((uint64_t*)readInData[j].data)[offset], true);
			
				gioReader->readData(i, false); // reading the whole file	
			}

			offset = offset + Np;
		}
		clock.stop();

		

		MPI_Barrier(comm);

		gioReader->close();
	}
	
	{
		int rangeX = minmaxX[1]-minmaxX[0];
		int rangeY = minmaxY[1]-minmaxY[0];
		int rangeZ = minmaxZ[1]-minmaxZ[0];

		mpiCartPartitions[0] = physScale[0]/rangeX;
		mpiCartPartitions[1] = physScale[1]/rangeY;
		mpiCartPartitions[2] = physScale[2]/rangeZ;

		if (myRank == 0)
			std::cout << "mpiCartPartitions: " << mpiCartPartitions[0] << ", " << mpiCartPartitions[1] << ", " << mpiCartPartitions[2] << std::endl;



		gio::GenericIO *gioWriter;

		// Create setup
		int periods[3] = { 0, 0, 0 };	
		MPI_Cart_create(comm, 3, mpiCartPartitions, periods, 0, &comm_cart);


		// Init GenericIO writer + open file
		gioWriter = new gio::GenericIO(comm_cart, _filename);
		gioWriter->setNumElems(totalNumberOfElements);


		// Init physical parameters
		for (int d = 0; d < 3; ++d)
		{
			gioWriter->setPhysOrigin(physOrigin[d], d);
			gioWriter->setPhysScale(physScale[d], d);
		}

		MPI_Barrier(comm_cart);
		// Populate parameters
		for (int i=0; i<readInData.size(); i++)
		{
			unsigned flag = gio::GenericIO::VarHasExtraSpace;
			if (readInData[i].xVar)
				flag |= gio::GenericIO::VarIsPhysCoordX;
			else if (readInData[i].yVar)
				flag |= gio::GenericIO::VarIsPhysCoordY;
			else if (readInData[i].zVar)
				flag |= gio::GenericIO::VarIsPhysCoordZ;


			if (readInData[i].dataType == "float")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (float*)readInData[i].data,    flag );
	        else if (readInData[i].dataType == "double")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (double*)readInData[i].data,   flag );
	        else if (readInData[i].dataType == "int8_t")
	          	gioWriter->addVariable(  (readInData[i].name).c_str(), (int8_t*)readInData[i].data,   flag);
	        else if (readInData[i].dataType == "int16_t")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (int16_t*)readInData[i].data,   flag);
	        else if (readInData[i].dataType == "int32_t")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (int32_t*)readInData[i].data,   flag);
	        else if (readInData[i].dataType == "int64_t")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (int64_t*)readInData[i].data,   flag);
	        else if (readInData[i].dataType == "uint8_t")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (uint8_t*)readInData[i].data,   flag);
	        else if (readInData[i].dataType == "uint16_t")
	          	gioWriter->addVariable(  (readInData[i].name).c_str(), (uint16_t*)readInData[i].data, flag);
	        else if (readInData[i].dataType == "uint32_t")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (uint32_t*)readInData[i].data,  flag);
	        else if (readInData[i].dataType == "uint64_t")
	          	gioWriter->addVariable( (readInData[i].name).c_str(), (uint64_t*)readInData[i].data,  flag);
	        else
	          	std::cout << " = data type undefined!!!" << std::endl;		
		}

		gioWriter->useOctree(numOctreeLevels);
		gioWriter->write();
		log << "HACCDataLoader::writeData " << _filename << std::endl;

		//MPI_Barrier(comm_cart);
		//gioWriter->close();

		if (myRank == 0)
			std::cout  << "HACCDataLoader::writeData done!"  << std::endl;
	}

	clock.stop();
	log << "Writing data took " << clock.getDuration() << " s" << std::endl;

	std::ofstream outputFile( (_filename  + "_" + std::to_string(myRank) + ".log").c_str(), std::ios::out);
	outputFile << log.str();
	outputFile.close();

	MPI_Barrier(comm);
	//MPI_Barrier(comm_cart);
	MPI_Comm_free(&comm_cart);
    comm_cart = MPI_COMM_NULL;

	MPI_Finalize();

	return 1;
}

// mpirun -np 8 ../genericio/mpi/GenericIORewriteOctree /bigData/Halos/b0168/m001-499.sodproperties m001-499.sodproperties-withOct 2