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

	std:string fileType;
	int octreeLevels;
	std::string maxFileCompression;
	bool prototypeIO;
	
	nlohmann::json parseJsonFile(std::string filename);
	int setFileType(std::string filetype); 
	std::string splitString(std::string theString, char delim);

	gio::GenericIO writer;

  public:
	IO(std::string _filetype, std::string filename, MPI_COMM comm);
	~IO(){};
	
	int setOctreeLevels(int levels);
	void setNumElements(size_t numElements) { writer.setNumElems(numElements) };
	void setPhysOrigin(double origin, int dim=-1){ writer.setPhysOrigin(origin, dim); }
	void setPhysScale(double origin, int dim=-1){ writer.setPhysScale(origin, dim); 

	template <typename T>
	int addVariable(std::string name, T *data, std::string compression="None", std::string otherParams="");
	int write();
};


inline IO::IO(std::string _filetype, std::string filename, MPI_COMM comm)
{
	prototypeIO = false;
	writer = new GenericIO(comm, filename);
	setFileType(filetype);
}


inline int IO::setFileType(std::string _filetype)
{
	filetype = _filetype;

	if (filetype == "prototype")
	{
		prototypeIO = true;
		return 1;
	}


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

		
		// Check Octree
		octreeON = false;
		if (fileInfo.contains("octee"))
		{
			if (fileInfo["octee"] == "Never")
				octreeLevels = -1;
			else if (fileInfo["octee"] == "Off")
				octreeLevels = 0;
			else if (fileInfo["octee"].is_number())
			{
				octreeLevels = fileInfo["octee"];
				octreeON = true;
			}
		}


		// Check Compression
		if (fileInfo.contains("max-compression-level"))
		{
			maxFileCompression = "Lossy";	// default
			if (fileInfo["max-compression-level"] == "None")
				maxFileCompression = "None";
			else if (fileInfo["max-compression-level"] == "Lossless")
				maxFileCompression = "Lossless";
			else if (fileInfo["max-compression-level"] == "Lossy")
				maxFileCompression = "Lossy";
		}

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
	if (octreeLevels != -1)
		octreeLevels = levels;

	return 1;
}


inline int IO::write()
{
	if (octreeLevels > 0)
		writer.useOctree(octreeLevels);

	writer.write();
}


inline std::string IO::splitString(std::string theString, char delim)
{
	std::vector<std::string> splits;

	int prev = 0;
	for (int i=0; i<theString.length(); i++)
	{
		if (theString[i] == delim)
		{
			splits.push_back( userCompressorSpecs.substr(prev, i-prev) );
			prev = i;
		}
	}

	return splits;
}




template <typename T>
inline int IO::addVariable(std::string name, T *data, std::string compression, std::string otherParams)
{
	if (prototypeIO)
	{
		writer.addVariable(name, data, flags, compressParams);
		return 1;
	}


	bool scalarFound;
	int scalarIndex;
	std::vector<std::string> params;


	// Check if filetype is valid
	if ( !fileInfo.contains("variables") )
	{
		std::cout << "The specified filetype " << fileType << " does not contain any variables. This could be a bug!" << std::endl;
		return -1;
	}


	// Find that scalar
	for (int i=0; i<fileInfo["variables"].size(); i++)
	{
		// Check if that variable exists and find type 
		if (fileInfo["variables"][i][0] == name)
		{
			scalarFound = true;
			scalarIndex = i;

			// Gather the other specs for that variable
			for (int j=1; j<fileInfo["variables"][i].size(); j++)
				params.push_back(fileInfo["variables"][i][j]);

			// done
			break;
		}
	}


	// Foiling attempt to add non existing scalar
	if (!scalarFound)
	{
		std::cout << "Varaible " << name << " does not exist for filetype " << fileType << std::endl;
		return -1;
	}
	

	//
	// Process parameters

	// Flags
	unsigned flags = 0;
	if ( fileInfo["variables"][scalarIndex].contains("CoordX") )
		flags = flags | GenericIO::VarIsPhysCoordX;
	else if ( fileInfo["variables"][scalarIndex].contains("CoordY") )
		flags = flags | GenericIO::VarIsPhysCoordY;
	else if ( fileInfo["variables"][scalarIndex].contains("CoordZ") )
		flags = flags | GenericIO::VarIsPhysCoordZ;
		
	if ( fileInfo["variables"][scalarIndex].contains("extra-space") )
		flags = flags | GenericIO::VarHasExtraSpace;


	/*
		IO_Layer::IO newGIO("standard-output", filename, MPI_Comm);
		newGIO.setOctreeLevels(3);
		newGIO.setNumElems(numParticles);

		for (int d=0; d<3; ++d)
        {
            newGIO.setPhysOrigin(physOrigin[d], d);
            newGIO.setPhysScale(physScale[d], d);
        }

		newGIO.addVariable("x", xx);
        newGIO.addVariable("y", yy);
		newGIO.addVariable("z", zz);
		newGIO.addVariable("vx", vx, "compress:SZ~mode:pw_rel 0.1");
        newGIO.addVariable("vy", vy, "compress:SZ~mode:pw_rel 0.1");
        newGIO.addVariable("vz", vz, "ccompress:SZ~mode:pw_rel 0.1");
        newGIO.addVariable("phi", phi, "compress:SZ~mode:pw_rel 0.003");
		newGIO.addVariable("id", id);
		newGIO.addVariable("mask", mask);

		newGIO.write();
	*/


	// No Compression
	if ( (maxFileCompression == "None") || (compression == "None"))
	{
		writer.addVariable(name, data, flags);
		return 1;
	}

	

	// Find Compression spefified in JSON spec file
	std::string dafaultCompressorSpecs = "";
	std::string maxCompressorSpecs = "";
	
	for (int i=0; i<fileInfo["variables"][scalarIndex].size(); i++)
	{
		std::string tempStr = fileInfo["variables"][scalarIndex][i];

		if (tempStr.compare(0, 16, "default-compressor:") == 0)
			dafaultCompressorSpecs = tempStr.substr(19, tempStr.length()-19);

		if (tempStr.compare(0, 16, "max-compression:") == 0)
			maxCompressorSpecs = tempStr.substr(16, tempStr.length()-16);
	}
	
	int maxCompressorSpecsLevel = 0;
	if (maxCompressorSpecs == "lossy" || maxCompressorSpecs == "")
		maxCompressorSpecsLevel = 2;
	else if (maxCompressorSpecs == "lossless")
		maxCompressorSpecsLevel = 1;
	else // None
		maxCompressorSpecsLevel = 0;


	// User specifies nothing, use default compressor specs
	if ( compression == "" )
	{
		writer.addVariable(name, data, flags, dafaultCompressorSpecs);
		return 1;
	}


	// Get user specified compression
	std::string userCompressorSpecs = "";
	userCompressorSpecs = compression.substr(9, userCompressorSpecs.length()-9);
	std::vector<std::string> userCompressParams = splitString(userCompressorSpecs,'~');


	if (userCompressParams[0] == "None")
		writer.addVariable(name, data, flags);
	else
		if (userCompressParams[0] == "SZ" && maxCompressorSpecsLevel == 2)	// User wants lossy and specs ok with that
			writer.addVariable(name, data, flags, userCompressorSpecs);
		else
			if (userCompressParams[0] == "BLOSC" && maxCompressorSpecsLevel >= 1)	// User wants lossy and specs ok with that
				writer.addVariable(name, data, flags, userCompressorSpecs);
			else
				writer.addVariable(name, data, flags, dafaultCompressorSpecs);

	return 1
	
}



} // namespace IO_Layer