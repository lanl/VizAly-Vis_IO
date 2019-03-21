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

	std::string serialize()
	{
		std::stringstream ss;
		ss << blockID << " : "
		  << minX << " - "
		  << maxX << ", "
		  << minY << " - "
		  << maxY << ", "
		  << minZ << " - "
		  << maxZ << ", "
		  << numParticles << ", "
		  << offsetInFile << ", "
		  << partitionLocation << std::endl;

    	return ss.str();
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
    		std::cout << rows[i].serialize();
    	std::cout << "\n"<< std::endl;
    }
};


struct PartitionExtents
{
	float extents[6];

	PartitionExtents(){};
	PartitionExtents(float _extents[6]){ for (int i=0; i<6; i++) extents[i]=_extents[i]; }

	std::string serialize()
	{ 
		std::stringstream ss;
		ss << extents[0] << " - " << extents[1] << ", " 
			<< extents[2] << " - " << extents[3] << ", " 
			<< extents[4] << " - " << extents[5] << std::endl; 

		return ss.str();
	}
};


struct Leaves
{
	std::vector<int> leafId;
};


class Octree
{
	int myRank;

	float extents[6];		// Extents of the HACC sim
	int maxSimExtents[3];	// border of the sim, usually 256

	int filePartition[3];	// Decomposition on the sim
	int numLevels;			// number of octree levels

	float rankExtents[6];	// extents of the current rank

	std::stringstream log;
	
  public:
  	

	Octree(){};
	Octree(int _myRank, float _rankExtents[6]):myRank(_myRank){ for (int i=0; i<6; i++) rankExtents[i]=_rankExtents[i]; };
	~Octree(){};

	void init(int _numLevels, float _extents[6], int xDiv, int yDiv, int zDiv);

	std::vector<float> getMyLeavesExtent(float myRankExtents[6], int numLevels);
	std::vector<PartitionExtents> ComputeMyLeaves(float myRankExtents[6], int _numLevels);
	
	template <typename T> void reorganizeArray(int numPartitions, std::vector<uint64_t>partitionCount, std::vector<int> partitionPosition, T array[], size_t numElements, bool shuffle);
	template <typename T> std::vector<uint64_t> findLeaf(T inputArrayX[], T inputArrayY[], T inputArrayZ[], size_t numElements, int numPartitions, float partitionExtents[], std::vector<int> &partitionPosition);

	template <typename T> bool checkPosition(float extents[], T _x, T _y, T _z);
	template <typename T> bool checkPositionInclusive(float extents[], T _x, T _y, T _z);

	std::string getLog();
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


	// Move to correct position
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
  		std::mt19937 g(0);	// to ensure reproducability

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

	// log << "\nOctree::reorganizeArray overall mem usage " << memCheck.getMemorySizeInMB() << " MB " << std::endl;
	// log << "Octree::reorganizeArray partitionOffset mem usage " << partitionOffsetMem.getMemorySizeInMB() << " MB " << std::endl;
	// log << "Octree::reorganizeArray currentPartitionCount mem usage " << currentPartitionCountMem.getMemorySizeInMB() << " MB " << std::endl;
	// log << "Octree::reorganizeArray tempVector mem usage " << tempVectorMem.getMemorySizeInMB() << " MB " << std::endl;
	// log << "Octree::reorganizeArray took " << clock.getDuration() << " s " << std::endl << std::endl;
}


template <typename T> 
inline std::vector<uint64_t> Octree::findLeaf(T inputArrayX[], T inputArrayY[], T inputArrayZ[], size_t numElements,
											 int numLeaves, float leavesExtents[], std::vector<int> &leafPosition)
{
	Timer clock;
	clock.start();

	Memory leafCountMem;
	leafCountMem.start();

	
	// Initialize count of leaf to 0
	std::vector<uint64_t>leafCount;		// # particles in leaf
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
			if (rankExtents[1] == maxSimExtents[0] && tempCoords[0] == 0)
				tempCoords[0] = maxSimExtents[0];

			if (rankExtents[3] == maxSimExtents[1] && tempCoords[1] == 0)
				tempCoords[1] = maxSimExtents[1];

			if (rankExtents[5] == maxSimExtents[2] && tempCoords[2] == 0)
				tempCoords[2] = maxSimExtents[2];


			// Move Cycled patciles at 0 border
			if (rankExtents[0] == 0 && tempCoords[0] == maxSimExtents[0])
				tempCoords[0] = 0;

			if (rankExtents[2] == 0 && tempCoords[1] == maxSimExtents[1])
				tempCoords[1] = 0;

			if (rankExtents[4] == 0 && tempCoords[2] == maxSimExtents[2])
				tempCoords[2] = 0;

			log << "\n" << myRank << " ~ my rank extents: " << rankExtents[0] << " - " << rankExtents[1] << ", "
									   		 					  << rankExtents[2] << " - " << rankExtents[3] << ", "
									   		 					  << rankExtents[4] << " - " << rankExtents[5] 
					  << "\n || Old Pos: " << inputArrayX[i] << ", " << inputArrayY[i] << ", " << inputArrayZ[i] 
					  << "\n || New Pos: " << tempCoords[0]  << ", " << tempCoords[1]  << ", " << tempCoords[2]<< std::endl; 
			
			for (l=0; l<numLeaves; l++)
				if ( checkPositionInclusive(&leavesExtents[l*6], tempCoords[0], tempCoords[1], tempCoords[2]) )
					break;
		}


		if (l >= numLeaves)
		{
			log << "\n" << myRank << " ~ " << inputArrayX[i] << ", " << inputArrayY[i] << ", " << inputArrayZ[i] << " is in NO partition!!! "
					  << "\n my rank extents: " << rankExtents[0] << " - " << rankExtents[1] << ", "
									   		 << rankExtents[2] << " - " << rankExtents[3] << ", "
									   		 << rankExtents[4] << " - " << rankExtents[5] << std::endl;
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




inline std::vector<float> Octree::getMyLeavesExtent(float myRankExtents[6], int numLevels)
{
	std::vector<PartitionExtents> myLeaves = ComputeMyLeaves(myRankExtents, numLevels);
	std::vector<float> leavesExtent;

	for (int l=0; l<myLeaves.size(); l++)
		for (int i=0; i<6; i++)
			leavesExtent.push_back(myLeaves[l].extents[i]);

	return leavesExtent;
}


inline std::vector<PartitionExtents> Octree::ComputeMyLeaves(float myRankExtents[6], int _numLevels)
{
	Timer clock;
	clock.start();

	// Create partitions
	PartitionExtents firstHalf, secondHalf;
	PartitionExtents currentRoot(myRankExtents);


	// Initialize rank with current partition
	std::list<PartitionExtents> currentPatitionList;
	currentPatitionList.push_back(currentRoot);


	//  Start
	int numDesiredBlocks = (int) pow(8.0f, _numLevels-1);	// Compute number of splits needed based on levels
	int splittingAxis = 0;									// start with x-axis
	int numBlocks = 1;

	while (numBlocks < numDesiredBlocks)
	{
		int numCurrentBlocks = currentPatitionList.size();

		for (int i=0; i<numCurrentBlocks; i++)
		{
			PartitionExtents temp = currentPatitionList.front();
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

	// Put list into vector
	std::vector<PartitionExtents> v{ std::begin(currentPatitionList), std::end(currentPatitionList) };


	clock.stop();
	log << "Octree::chopVolume took " << clock.getDuration() << " s " << std::endl;

	return v;
}










inline void writeLog(std::string filename, std::string log)
{
	std::ofstream outputFile( (filename+ ".log").c_str(), std::ios::out);
	outputFile << log;
	outputFile.close();
}


inline std::string Octree::getLog()
{ 
	std::string currentLog = log.str();
	log.str("");
	return currentLog;
}

#endif
