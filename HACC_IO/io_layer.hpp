#pragma once

#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>

#include "../genericio/GenericIO.h"

#include "utils/json.hpp"

namespace IO_Layer {


class IO
{
	nlohmann::json fileInfo;
	int octreeLevels;
	std::string maxCompressionType;
	bool prototypeIO;
	
	nlohmann::json parseJsonFile(std::string filename);
	int setFileType(std::string filetype); 

	gio::GenericIO writer;

  public:
	IO(std::string filetype, std::string filename, MPI_COMM comm);
	~IO(){};
	
	int setOctreeLevels(int levels);
	void setNumElements(size_t numElements);
	void setPhysOrigin(double origin, int dim=-1){ writer.setPhysOrigin(origin, dim); }
	void setPhysScale(double origin, int dim=-1){ writer.setPhysScale(origin, dim); 

	template <typename T>
	int addVariable(std::string name, T *data, std::string compression="None", std::string otherParams="");
	int write();
};


inline IO::IO(std::string filetype, std::string filename, MPI_COMM comm)
{
	prototypeIO = false;
	writer = new GenericIO(comm, filename);
	setFileType(filetype);
}


inline int IO::setFileType(std::string filetype)
{
	// Read in the different types of file formats that exist
	nlohmann::json jsonInput = parseJsonFile("input_specs/file-types.json");
	nlohmann::json data = jsonInput["hacc-file-types"];
	std::vector<std::string> filetypes;
	for (auto it=data.begin(); it!=data.end(); ++it)
		filetypes.push_back( it.value() );


	// Check to see if that filetype is legit
	auto found = std::find(filetypes.begin(), filetypes.end(), filetype);
	if (found != filetypes.end())
	{
		// pull in the definition of the file
		fileInfo = parseJsonFile("input_specs/" + filetype + ".json");

		// Check Compression
		if (fileInfo.contains("max-compression-level"))
		{
			maxCompressionType = "None";
			if (fileInfo["max-compression-level"] == "Lossless")
				maxCompressionType = "Lossless";
			else if (fileInfo["max-compression-level"] == "Lossy")
				maxCompressionType = "Lossy";
		}

		// Set Octree
		octreeON = false;
		if (fileInfo.contains("octee"))
		{
			if (fileInfo["octee"] == "Never")
				octreeLevels = 0;
			else if (fileInfo["octee"] == "No")
				octreeLevels = 0;
			else if (fileInfo["octee"] == "Yes")
				octreeLevels = 4;
		}

		if (filetype == "prototype")
			prototypeIO = true;

		return 1;
	}
	else
		return 0;	// failed
}


inline int IO::setOctreeLevels(int levels)
{ 
	if (prototypeIO)
	{
		octreeLevels = levels;
		return 1;
	}


	// Only allow octree to be on if the specs allow for it
	if (fileInfo["octee"] == "Never")
		octreeLevels = 0;
	else if (fileInfo["octee"] == "Off")
		octreeLevels = 0;
	else if (fileInfo["octee"] == "On")
		octreeLevels = levels;

	return 1;
}


template <typename T>
inline int IO::addVariable(std::string name, T *data, std::string compressionType="None", std::string otherParams="")
{
	if (prototypeIO)
	{

		return 1;
	}


	bool scalarFound;
	int scalarIndex;
	std::string dataType;
	std::vector<std::string> params;

	if (fileInfo.contains("scalars"))
	{
		for (int i=0; i<fileInfo["scalars"].size(); i++)
		{
			// Check if that variable exists and find type 
			if (fileInfo["scalars"][i][0] == name)
			{
				scalarFound = true;
				scalarIndex = i;
				dataType = fileInfo["scalars"][i][1];
			}

			// Gather the other specs for that variable
			if (scalarFound)
			{
				for (int j=2; j<fileInfo["scalars"][i].size(); j++)
					params.push_back(fileInfo["scalars"][i][j]);

				break;
			}
		}
	}
	else
		return -1;

	// Process parameters
	if (scalarFound)
	{
		// Flags
		unsigned flags = 0;
		if ( fileInfo["scalars"][scalarIndex].contains("CoordX") )
			flags = flags | GenericIO::VarIsPhysCoordX;
		else if ( fileInfo["scalars"][scalarIndex].contains("CoordY") )
			flags = flags | GenericIO::VarIsPhysCoordY;
		else if ( fileInfo["scalars"][scalarIndex].contains("CoordZ") )
			flags = flags | GenericIO::VarIsPhysCoordZ;
			
		if ( fileInfo["scalars"][scalarIndex].contains("extra-space") )
			flags = flags | GenericIO::VarHasExtraSpace;


		std::cout << "Flags: " << flags << std::endl;


		if ( (maxCompressionType == "None") || (compressionType == "None"))
		{
			writer.addVariable(name, data, flags);
			std::cout << "No Compression" << std::endl;
		}
		else
		{	// Compression is allowed and was probably asked for

			if (compressionType.compare(0, 9, "compress:") == 0)
			{
				// Compression was requested

				if (compressionType.compare(9, 5, "BLOSC") == 0)	// BLOSC was requested
				{
					writer.addVariable(name, data, flags, "BLOSC");
					std::cout << "BLOSC" << std::endl;
				}
				else if (compressionType.compare(9, 2, "SZ") == 0)	// Lossy SZ was requested
				{
					// Check what the specs mentioned
					for (int i=0; i<fileInfo["scalars"][scalarIndex].size(); i++)
					{
						std::string tempStr =  fileInfo["scalars"][scalarIndex][i];
						if (tempStr.compare(0, 22, "max-compression-level:") == 0)
						{
							if (tempStr.compare(22, 8, "Lossless") == 0)	// Specs says lossless at best
							{
								std::cout << "No Compression" << std::endl;
								writer.addVariable(name, data, flags);		// hence, NO compression 
							}
							else 
							{
								// Compress lossily

								int stringLength = compressionType.length();
								std::string compressParams = compressionType.substr(9,stringLength-9);
								writer.addVariable(name, data, flags, compressParams);
							}
						}
					}
				}
				else
				{
					// Unknown parameter, we are not compressing!
					writer.addVariable(name, data, flags);
				}
			}
		}
	}
}


inline int IO::write()
{
	if (octreeLevels > 0)
		writer.useOctree(octreeLevels);

	writer.write();
}


} // namespace IO_Layer