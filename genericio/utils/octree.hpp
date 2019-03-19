#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <list>
#include <vector>
#include <math.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <stdio.h>
//#include <mpi.h>

#include "memory.h"
#include "timer.h"

struct GIOOctreeRow
{
    uint64_t blockID;
    uint64_t minX;
    uint64_t maxX;
    uint64_t minY;
    uint64_t maxY;
    uint64_t minZ;
    uint64_t maxZ;
    uint64_t numParticles;
    uint64_t offsetInFile;
    uint64_t partitionLocation;

	GIOOctreeRow(){ } ;
	GIOOctreeRow(uint64_t id, uint64_t _minX, uint64_t _maxX, uint64_t _minY, uint64_t _maxY,
				uint64_t _minZ, uint64_t _maxZ , uint64_t _numP, uint64_t _offsetInFile, uint64_t _parLocation)
	{
		blockID = id;
		minX = _minX; maxX = _maxX;
		minY = _minY; maxY = _maxY;
		minZ = _minZ; maxZ = _maxZ;
		numParticles = _numP;
		offsetInFile = _offsetInFile;
		partitionLocation = _parLocation;
	}
};


struct GIOOctree
{
    uint64_t preShuffled;			// particles shuffeed in leaves or not
    uint64_t decompositionLevel;	// 
    uint64_t numEntries;			// # of octree leaves
    std::vector<GIOOctreeRow> rows;

    std::string serialize(bool bigEndian)
    {
        std::stringstream ss;

        ss << serialize_uint64(preShuffled, bigEndian);
        ss << serialize_uint64(decompositionLevel, bigEndian);
        ss << serialize_uint64(numEntries, bigEndian);


        for (int i=0; i<numEntries; i++)
        {
            ss << serialize_uint64(rows[i].blockID, bigEndian);
            ss << serialize_uint64(rows[i].minX, bigEndian);
            ss << serialize_uint64(rows[i].maxX, bigEndian);
            ss << serialize_uint64(rows[i].minY, bigEndian);
            ss << serialize_uint64(rows[i].maxY, bigEndian);
            ss << serialize_uint64(rows[i].minZ, bigEndian);
            ss << serialize_uint64(rows[i].maxZ, bigEndian);
            ss << serialize_uint64(rows[i].numParticles, bigEndian);
            ss << serialize_uint64(rows[i].offsetInFile, bigEndian);
            ss << serialize_uint64(rows[i].partitionLocation, bigEndian);
        }
        
        return ss.str();
    }


    void deserialize(char *serializedString, bool bigEndian)
    {
        preShuffled 		= deserialize_uint64(&serializedString[0],  bigEndian);
        decompositionLevel 	= deserialize_uint64(&serializedString[8], bigEndian);
        numEntries 			= deserialize_uint64(&serializedString[16], bigEndian);
		
        rows.clear();
        int serializedOffset = 24;	// adding togehter the previous header

		
        for (int i=0; i<numEntries; i++)
        {
        	GIOOctreeRow temp;
			
            temp.blockID 	= deserialize_uint64(&serializedString[serializedOffset + 0], 		bigEndian);
            temp.minX 		= deserialize_uint64(&serializedString[serializedOffset + 8], 		bigEndian);
            temp.maxX 		= deserialize_uint64(&serializedString[serializedOffset + 16], 		bigEndian);
            temp.minY 		= deserialize_uint64(&serializedString[serializedOffset + 24], 		bigEndian);
            temp.maxY 		= deserialize_uint64(&serializedString[serializedOffset + 32], 		bigEndian);
            temp.minZ 		= deserialize_uint64(&serializedString[serializedOffset + 40], 		bigEndian);
            temp.maxZ 		= deserialize_uint64(&serializedString[serializedOffset + 48], 		bigEndian);
            temp.numParticles = deserialize_uint64(&serializedString[serializedOffset + 56], 	bigEndian);
            temp.offsetInFile = deserialize_uint64(&serializedString[serializedOffset + 64], 	bigEndian);
            temp.partitionLocation = deserialize_uint64(&serializedString[serializedOffset + 72], bigEndian);
			
            serializedOffset += 80;
            rows.push_back(temp);
			
        }
    }



	uint64_t deserialize_uint64(char* serializedStr, bool bigEndian)
	{
		uint64_t num;

		if (!bigEndian)
		{
		 	num =   	
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[0])) << 0 ) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[1])) << 8 ) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[2])) << 16) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[3])) << 24) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[4])) << 32) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[5])) << 40) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[6])) << 48) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[7])) << 56) ;
		}
		else
		{
			// Big Endian
			num =   	
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[0])) << 0 ) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[1])) << 8 ) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[2])) << 16) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[3])) << 24) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[4])) << 32) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[5])) << 40) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[6])) << 48) |
			(static_cast<uint64_t>(static_cast<uint8_t>( serializedStr[7])) << 56) ;
		}

		return num;
	}

	std::string serialize_uint64(uint64_t t, bool bigEndian)
	{
		std::vector<char> serializedString;
		if (!bigEndian)
		{
			serializedString.push_back( static_cast<uint8_t>((t >> 0) & 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 8) & 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 16)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 24)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 32)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 40)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 48)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 56)& 0xff) );
		}
		else
		{
			serializedString.push_back( static_cast<uint8_t>((t >> 56)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 48)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 40)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 32)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 24)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 16)& 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 8) & 0xff) );
			serializedString.push_back( static_cast<uint8_t>((t >> 0) & 0xff) );
		}

		std::stringstream ss;
		for (int i=0; i<8; i++)
			ss << serializedString[i];

		return ss.str();
	}


    void print()
    {
    	std::cout << "\nPre-Shuffled (0=No, 1=Yes): " << preShuffled << std::endl;
    	std::cout << "Decomposition Level: " << decompositionLevel << std::endl;
    	std::cout << "Num Entries: " << numEntries << std::endl;

    	std::cout << "\nIndex : minX - maxX, minY - maxY, minZ - maxZ, #particles, offset in file, rank location"<< std::endl;
    	for (int i=0; i<numEntries; i++)
    	{
    		std::cout << rows[i].blockID << " : "
    				  << rows[i].minX << " - "
    				  << rows[i].maxX << ", "
    				  << rows[i].minY << " - "
    				  << rows[i].maxY << ", "
    				  << rows[i].minZ << " - "
    				  << rows[i].maxZ << ", "
    				  << rows[i].numParticles << ", "
    				  << rows[i].offsetInFile << ", "
    				  << rows[i].partitionLocation << std::endl;
    	}
    	std::cout << "\n"<< std::endl;
    }
};


struct PartitionExtents
{
	float extents[6];

	PartitionExtents(){};
	PartitionExtents(float _extents[6]){ for (int i=0; i<6; i++) extents[i]=_extents[i]; }

	void print()
	{ 
		std::cout << extents[0] << " - " << extents[1] << ", " 
			      << extents[2] << " - " << extents[3] << ", " 
			      << extents[4] << " - " << extents[5] << std::endl; 
	}
};

struct Leaves
{
	std::vector<int> leafId;
};

class Octree
{
	float extents[6];		// Extents of the HACC sim
	int maxSimExtents[3];	// border of the sim, usually 256

	int filePartition[3];	// Decomposition on the sim
	int numLevels;			// number of octree levels

	std::vector<Leaves> rankLeaf;	// Specify which leaves are in each rank

	float rankExtents[6];	// extents of the current rank ???

	void chopVolume(float rootExtents[6], int _numLevels, int partitions[3], std::list<PartitionExtents> & partitionList);

	std::stringstream log;
	
  public:
  	std::vector<PartitionExtents> octreePartitions;		// stores only extents of all leaves
  	int myRank;

	Octree(){};
	Octree(int _numLevels, float _extents[6]):numLevels(_numLevels){ for (int i=0; i<6; i++) extents[i] = _extents[i]; };
	~Octree(){};
	
	void init(int _numLevels, float _extents[6], int xDiv, int yDiv, int zDiv);
	void buildOctree();


	int getNumNodes(){ return octreePartitions.size(); }
	int getLeafIndex(float pos[3]);
	void getLeafExtents(int leafId, float extents[6]);
	void setRankExtents(float _rankExtents[6]){ for (int i=0;i<6;i++) rankExtents[i]=_rankExtents[i]; }
	std::vector<int> getLeaves(int rank){ return rankLeaf[rank].leafId; }

	std::string getPartitions();
	void displayPartitions();
	std::string getLog(){ return log.str(); }

	template <typename T> void fillArray(int numPartitions, std::vector<int>partitionCount, T array[], size_t numElements);
	template <typename T> void reorganizeArray(int numPartitions, std::vector<uint64_t>partitionCount, std::vector<int> partitionPosition, T array[], size_t numElements, bool shuffle);
	template <typename T> bool checkPosition(float extents[], T _x, T _y, T _z);
	template <typename T> bool checkPositionInclusive(float extents[], T _x, T _y, T _z);
	template <typename T> bool checkOverlap(T extents1[], T extents2[]);;
	template <typename T> std::vector<uint64_t> findLeaf(T inputArrayX[], T inputArrayY[], T inputArrayZ[], size_t numElements, int numPartitions, float partitionExtents[], std::vector<int> &partitionPosition);
};


inline void Octree::init(int _numLevels, float _extents[6], int xDiv, int yDiv, int zDiv)
{
	numLevels = _numLevels;

	for (int i=0; i<6; i++) 
		extents[i] = _extents[i];

	filePartition[0] = xDiv;
	filePartition[1] = yDiv;
	filePartition[2] = zDiv;

	maxSimExtents[0] = (int)( _extents[1] + 0.49);		// x
	maxSimExtents[1] = (int)( _extents[3] + 0.49);		// y
	maxSimExtents[2] = (int)( _extents[5] + 0.49);		// z
}


inline void Octree::buildOctree()
{
	std::list<PartitionExtents> _octreePartitions;
	chopVolume(extents, numLevels, filePartition, _octreePartitions);

	octreePartitions.resize( _octreePartitions.size() );
	std::copy(_octreePartitions.begin(), _octreePartitions.end(), octreePartitions.begin());


	if (myRank == 0)
	{
		std::cout << "Octree partitions: \n";
		displayPartitions();


		for (int i=0; i<rankLeaf.size(); i++)
		{
			std::cout << "Rank: " << i << std::endl;
			for (int l=0; l<rankLeaf[i].leafId.size(); l++)
				std::cout << rankLeaf[i].leafId[l] << ", ";
			std::cout << "\n";
		}
	}
}



template <typename T> 
inline bool Octree::checkPosition(float extents[], T _x, T _y, T _z)
{
	if (_x < extents[1] && _x >= extents[0])
		if (_y < extents[3] && _y >= extents[2])
			if (_z < extents[5] && _z >= extents[4])
				return true;

	return false;
}

template <typename T> 
inline bool Octree::checkPositionInclusive(float extents[], T _x, T _y, T _z)
{
	if (_x <= extents[1] && _x >= extents[0])
		if (_y <= extents[3] && _y >= extents[2])
			if (_z <= extents[5] && _z >= extents[4])
				return true;

	return false;
}

template <typename T> 
inline bool Octree::checkOverlap(T extents1[], T extents2[])
{
	if (extents1[1] <= extents2[0] || extents1[0] >= extents2[1])
		return false;

	if (extents1[3] <= extents2[2] || extents1[2] >= extents2[3])
		return false;

	if (extents1[5] <= extents2[4] || extents1[4] >= extents2[5])
		return false;

	return true;
}


template <typename T>				    	
inline void Octree::fillArray(int numPartitions, std::vector<int>partitionCount, T array[], size_t numElements)
{
	Timer clock;
	clock.start();

	size_t count = 0;
	for (int i=0; i<numPartitions; i++)
	{
		for (int j=0; j<partitionCount[i]; j++)
		{
			array[count] = i;
			count++;
		}
	}

	clock.stop();
	log << "Octree::fillArray took " << clock.getDuration() << " s " << std::endl;
}





// Main one
template <typename T>				    	
inline void Octree::reorganizeArray(int numPartitions, std::vector<uint64_t>partitionCount, 
									std::vector<int> partitionPosition, T array[], size_t numElements, bool shuffle)
{
	Timer clock;
	clock.start();

	Memory memCheck, partitionOffsetMem, currentPartitionCountMem, tempVectorMem;

  memCheck.start();
	

	// Get offset
  partitionOffsetMem.start();
	std::vector<int> partitionOffset;
	partitionOffset.push_back(0);
	for (int i=0; i<numPartitions; i++)
		partitionOffset.push_back( partitionOffset[i] + partitionCount[i] );

  partitionOffsetMem.stop();


	// Partition count
  currentPartitionCountMem.start();
	std::vector<int> currentPartitionCount;
	for (int i=0; i<numPartitions; i++)
		currentPartitionCount.push_back(0);
  currentPartitionCountMem.stop();


	// Duplicating original data array
  tempVectorMem.start();
	std::vector<T> tempVector(numElements);
	std::copy(array, array+numElements, tempVector.begin());


	for (size_t i=0; i<numElements; i++)
	{
		int partition = partitionPosition[i];	// Get the partition that index is in
		int pos = partitionOffset[partition] + currentPartitionCount[partition];	// current_new_position = offset + current

		array[pos] = tempVector[i];
		currentPartitionCount[partition]++;
	}
  tempVectorMem.stop();

  	// Clear memory ASAP
  	tempVector.clear();				tempVector.shrink_to_fit();
  	currentPartitionCount.clear();	currentPartitionCount.shrink_to_fit();
  	
  	

	if (shuffle)
	{
		std::random_device rd;
  		std::mt19937 g(rd());

  		size_t startPos = 0;
		for (int p=0; p<numPartitions; p++)
		{
			std::shuffle(&array[startPos], &array[startPos+partitionCount[p]], g);
			startPos += partitionCount[p];
		}
	}

	partitionCount.clear();	partitionCount.shrink_to_fit();

	memCheck.stop();
	clock.stop();

	memCheck, partitionOffsetMem, currentPartitionCountMem, tempVectorMem;


	log << "Octree::reorganizeArray overall mem usage " << memCheck.getMemorySizeInMB() << " MB " << std::endl;
	log << "Octree::reorganizeArray partitionOffset mem usage " << partitionOffsetMem.getMemorySizeInMB() << " MB " << std::endl;
	log << "Octree::reorganizeArray currentPartitionCount mem usage " << currentPartitionCountMem.getMemorySizeInMB() << " MB " << std::endl;
	log << "Octree::reorganizeArray tempVector mem usage " << tempVectorMem.getMemorySizeInMB() << " MB " << std::endl;
	log << "Octree::reorganizeArray took " << clock.getDuration() << " s " << std::endl;
}


template <typename T> 
inline std::vector<uint64_t> Octree::findLeaf(T inputArrayX[], T inputArrayY[], T inputArrayZ[], 
											size_t numElements, int numLeaves, float leavesExtents[], std::vector<int> &leafPosition)
{
	Timer clock;
	clock.start();

	Memory leafCountMem;
	leafCountMem.start();

	std::vector<uint64_t>leafCount;		// # particles in leaf

	//if (myRank == 0)
	//	std::cout <<  "numLeaves: " << numLeaves << std::endl;

	// Initialize count of leaf
	for (int l=0; l<numLeaves; l++)
		leafCount.push_back(0);


	// Find leaf position for each element in the rank
	for (size_t i=0; i<numElements; i++)
	{
		// leavesExtents[] - minX, maxY  minY, maxY, minZ, maxZ
		int l;
		for (l=0; l<numLeaves; l++)
			if ( checkPosition(&leavesExtents[l*6], inputArrayX[i], inputArrayY[i], inputArrayZ[i]) )
				break;


		// double check with less or equal
		if (l >= numLeaves)
			for (l=0; l<numLeaves; l++)
				if ( checkPositionInclusive(&leavesExtents[l*6], inputArrayX[i], inputArrayY[i], inputArrayZ[i]) )
					break;

		//
		// triple check for particles that cycle; instead of being 256 they are 0 and vice versa
		if (l >= numLeaves)
		{
			// Duplicate leaf coordinates so as not to change position
			std::vector<T> tempCoords;
			tempCoords.push_back( inputArrayX[i] );
			tempCoords.push_back( inputArrayY[i] );
			tempCoords.push_back( inputArrayZ[i] );

			// Move Cycled patciles at 256 border
			if (rankExtents[1] == 256 && tempCoords[0] == 0)
				tempCoords[0] = 256;

			if (rankExtents[3] == 256 && tempCoords[1] == 0)
				tempCoords[1] = 256;

			if (rankExtents[5] == 256 && tempCoords[2] == 0)
				tempCoords[2] = 256;


			// Move Cycled patciles at 0 border
			if (rankExtents[0] == 0 && tempCoords[0] == 256)
				tempCoords[0] = 0;

			if (rankExtents[2] == 0 && tempCoords[1] == 256)
				tempCoords[1] = 0;

			if (rankExtents[4] == 0 && tempCoords[2] == 256)
				tempCoords[2] = 0;

			std::cout << "my rank extents: " << rankExtents[0] << "-" << rankExtents[1] << ", "
									   		 << rankExtents[2] << "-" << rankExtents[3] << ", "
									   		 << rankExtents[4] << "-" << rankExtents[5] 
					  << " Old Pos: " << inputArrayX[i] << ", " << inputArrayY[i] << ", " << inputArrayZ[i] 
					  << " New Pos: " << tempCoords[0]  << ", " << tempCoords[1]  << ", " << tempCoords[2]<< std::endl; 
			
			for (l=0; l<numLeaves; l++)
				if ( checkPositionInclusive(&leavesExtents[l*6], tempCoords[0], tempCoords[1], tempCoords[2]) )
					break;
		}


		if (l >= numLeaves)
		{
			std::cout << "\n" << myRank << " ~ " << inputArrayX[i] << ", " << inputArrayY[i] << ", " << inputArrayZ[i] << " is in NO partition!!! " << std::endl;
			std::cout << "my rank extents: " << rankExtents[0] << "-" << rankExtents[1] << ", "
									   		 << rankExtents[2] << "-" << rankExtents[3] << ", "
									   		 << rankExtents[4] << "-" << rankExtents[5] << std::endl;
			// Put it in the last partition
			l=numLeaves-1;
		}
		
		// Put in leaf
		leafPosition.push_back(l);
		leafCount[l]++;
	}

	leafCountMem.stop();
	clock.stop();
	log << "Octree::findLeaf took " << clock.getDuration() << " s " << std::endl;
	log << "Octree::findLeaf leafCount mem usage " << leafCountMem.getMemorySizeInMB() << " MB " << std::endl;

	return leafCount;
}


inline void Octree::chopVolume(float rootExtents[6], int _numLevels, int partitions[3], std::list<PartitionExtents> & partitionList)
{
	Timer clock;
	clock.start();

	//
	// Set the first partition as root 
	int numOctreePartitions = 1;
	PartitionExtents temp(rootExtents);				
	partitionList.push_back(temp);

	// Set leaves
	rankLeaf.resize(1);
	rankLeaf[0].leafId.push_back(0);

	if (_numLevels == 0)
		return;


	//
	// Set the second partition as rank extents in file

	// Remove first partition
	partitionList.pop_front();
	numOctreePartitions = 0;

	// Get extensts of the file
	float xExtents = rootExtents[1]-rootExtents[0];
	float yExtents = rootExtents[3]-rootExtents[2];
	float zExtents = rootExtents[5]-rootExtents[4];


	// Create partitions
	for (int x_axis=0; x_axis<partitions[0]; x_axis++)
		for (int y_axis=0; y_axis<partitions[1]; y_axis++)
			for (int z_axis=0; z_axis<partitions[2]; z_axis++)
			{
				float currentPartition[6];

				currentPartition[0] = xExtents/partitions[0] * x_axis;
				currentPartition[1] = currentPartition[0] + xExtents/partitions[0];

				currentPartition[2] = yExtents/partitions[1] * y_axis;
				currentPartition[3] = currentPartition[2] + yExtents/partitions[1];

				currentPartition[4] = zExtents/partitions[2] * z_axis;
				currentPartition[5] = currentPartition[4] + zExtents/partitions[2];

				PartitionExtents tempRank(currentPartition);
				partitionList.push_back(tempRank);

				numOctreePartitions++;
			}


	// Set leaves
	rankLeaf.clear();
	rankLeaf.resize(numOctreePartitions);
	for (int i=0; i<numOctreePartitions; i++)
		rankLeaf[i].leafId.push_back(i);

	if (_numLevels == 1)
		return;



	//
	// Now start diving the ranks
	int _numRanks = numOctreePartitions;

	// Copy data from current partition list to rank list
	std::list<PartitionExtents> rankList;
	for (int r=0; r<_numRanks; r++)
	{
		temp = partitionList.front();
		rankList.push_back(temp);
		partitionList.pop_front();
	}


	// Set leaves
	rankLeaf.clear();
	rankLeaf.resize(_numRanks);


	// Partitions
	numOctreePartitions = 0;
	for (int r=0; r<_numRanks; r++)
	{
		// Get current root we are partitioning
		PartitionExtents currentRoot = rankList.front();
		rankList.pop_front();


		// Initialize rank with current partition
		std::list<PartitionExtents> currentPatitionList;
		currentPatitionList.push_back(currentRoot);


		//
		//  Stat
		PartitionExtents firstHalf, secondHalf;

		int splittingAxis = 0;									// start with x-axis
		int numDesiredBlocks = (int) pow(8.0f, _numLevels-1);	// Compute number of splits needed based on levels
		int numBlocks = 1;

		while (numBlocks < numDesiredBlocks)
		{
			int numCurrentBlocks = currentPatitionList.size();

			for (int i=0; i<numCurrentBlocks; i++)
			{
				temp = currentPatitionList.front();
				currentPatitionList.pop_front();

				if (splittingAxis == 0)			// x-axis
				{
					firstHalf.extents[0] =  temp.extents[0];
					firstHalf.extents[1] = (temp.extents[0] + temp.extents[1])/2;

					secondHalf.extents[0] = (temp.extents[0] + temp.extents[1])/2;
					secondHalf.extents[1] =  temp.extents[1];


					firstHalf.extents[2] = secondHalf.extents[2] = temp.extents[2];
					firstHalf.extents[3] = secondHalf.extents[3] = temp.extents[3];

					firstHalf.extents[4] = secondHalf.extents[4] = temp.extents[4];
					firstHalf.extents[5] = secondHalf.extents[5] = temp.extents[5];
				}
				else
					if (splittingAxis == 1)		// y-axis
					{
						firstHalf.extents[0] = secondHalf.extents[0] = temp.extents[0];
						firstHalf.extents[1] = secondHalf.extents[1] = temp.extents[1];


						firstHalf.extents[2] =  temp.extents[2];
						firstHalf.extents[3] = (temp.extents[2] + temp.extents[3])/2;

						secondHalf.extents[2] = (temp.extents[2] + temp.extents[3])/2;
						secondHalf.extents[3] =  temp.extents[3];


						firstHalf.extents[4] = secondHalf.extents[4] = temp.extents[4];
						firstHalf.extents[5] = secondHalf.extents[5] = temp.extents[5];
					}
					else
						if (splittingAxis == 2)	// z-axis
						{
							firstHalf.extents[0] = secondHalf.extents[0] = temp.extents[0];
							firstHalf.extents[1] = secondHalf.extents[1] = temp.extents[1];

							firstHalf.extents[2] = secondHalf.extents[2] = temp.extents[2];
							firstHalf.extents[3] = secondHalf.extents[3] = temp.extents[3];


							firstHalf.extents[4] =  temp.extents[4];
							firstHalf.extents[5] = (temp.extents[4] + temp.extents[5])/2;

							secondHalf.extents[4] = (temp.extents[4] + temp.extents[5])/2;
							secondHalf.extents[5] =  temp.extents[5];
						}

				currentPatitionList.push_back(firstHalf);
				currentPatitionList.push_back(secondHalf);
			}

			// cycle axis
			splittingAxis++;
			if (splittingAxis == 3)
				splittingAxis = 0;

			numBlocks = currentPatitionList.size();
		}


		// Fill partion list with items from this rank
		for (auto it=currentPatitionList.begin(); it!=currentPatitionList.end(); ++it)
		{
			rankLeaf[r].leafId.push_back( partitionList.size() );

			partitionList.push_back( *it );
			numOctreePartitions++;
		}
	}

	clock.stop();
	log << "Octree::chopVolume took " << clock.getDuration() << " s " << std::endl;
}



inline void Octree::displayPartitions()
{
	int count = 0;
	for (auto it=octreePartitions.begin(); it!=octreePartitions.end(); it++, count++)
		std::cout << count << " : " << (*it).extents[0] << ", " << (*it).extents[1] <<  "   "
				  					<< (*it).extents[2] << ", " << (*it).extents[3] <<  "   "
				  					<< (*it).extents[4] << ", " << (*it).extents[5] <<  std::endl;
}


inline std::string Octree::getPartitions()
{
	std::string outputString = "";
	int count = 0;
	for (auto it=octreePartitions.begin(); it!=octreePartitions.end(); it++, count++)
		outputString += std::to_string(count) + " : " + std::to_string((*it).extents[0]) + ", " + std::to_string((*it).extents[1]) + "  "
				  									  + std::to_string((*it).extents[2]) + ", " + std::to_string((*it).extents[3]) + "  "
				  									  + std::to_string((*it).extents[4]) + ", " + std::to_string((*it).extents[5]) + "\n";
	return outputString;
}


inline void Octree::getLeafExtents( int leafId, float _extents[6])
{
	int count = 0;
	for (int i=0; i<6; i++)
		_extents[i] = octreePartitions[leafId].extents[i];
}


inline int Octree::getLeafIndex(float pos[3])
{
	float x = pos[0];
	float y = pos[1];
	float z = pos[2];
	for (int i=0; i<octreePartitions.size(); i++)
	{		

		// Nothing should be at 256, but some are!!!
		if (x >= maxSimExtents[0]) x = (float)maxSimExtents[0] - 0.00001;
		if (y >= maxSimExtents[1]) y = (float)maxSimExtents[1] - 0.00001;
		if (z >= maxSimExtents[2]) z = (float)maxSimExtents[2] - 0.00001;

		if (x >= extents[0] && x<extents[1])
			if (y >= extents[2] && y<extents[3])
				if (z >= extents[4] && z<extents[5])
					return i;	
	}

	return -1;
}


inline void writeLog(std::string filename, std::string log)
{
	std::ofstream outputFile( (filename+ ".log").c_str(), std::ios::out);
	outputFile << log;
	outputFile.close();
}

#endif
