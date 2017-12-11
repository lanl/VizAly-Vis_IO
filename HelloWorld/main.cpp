#include <mpi.h>
#include <iostream>

int main(int argc, char** argv)
{
	MPI_Init(NULL, NULL);

	int numRanks, myRank;
	MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	// Get the name of the processor
	char procName[MPI_MAX_PROCESSOR_NAME];
	int nameLength;
	MPI_Get_processor_name(procName, &nameLength);

	
	#ifndef NDEBUG
		std::cout << "Hello (Debug mode) from " << procName << ", rank: " << myRank << " out of " << numRanks << " ranks." << std::endl;
	#else 
		// For CMAKE_BUILD_TYPE: Release
		std::cout << "Hello (NON Debug mode) from " << procName << ", rank: " << myRank << " out of " << numRanks << " ranks." << std::endl;
	#endif

	MPI_Finalize();
}

// Running
// mpirun -np 8 ./HelloWorld
