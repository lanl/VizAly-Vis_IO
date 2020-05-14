#pragma once

#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>

#include "../genericio/GenericIO.h"

#include "utils/json.hpp"
#include "input_specs/filetypes.h"


namespace IO_Layer {

class IO
{
	MPI_Comm myComm;
	std::string filetype;				// filetype: checkpoint, fof, ...
	int octreeLevels;					// number of octree levels: 0 = no octree
	nlohmann::json fileInfo;			// information about the file being written
	std::string maxFileCompression;		// compression level allowed for file
	bool prototypeIO;					// whether it's a custom file
	gio::GenericIO *writer;				// handle to GenericIO

	std::stringstream log;				// log

	int setFileType(std::string filetype); 
	std::vector<std::string> splitString(std::string const &str, const char* delim);
	void writeLogToDisk(std::string logFilename, std::string logContents);

  public:

	IO(std::string _filetype, std::string filename, MPI_Comm comm);
	~IO(){};
	
	void setNumElements(size_t numElements) { writer->setNumElems(numElements); };
	void setPhysOrigin(double origin, int dim=-1){ writer->setPhysOrigin(origin, dim); }
	void setPhysScale(double origin, int dim=-1){ writer->setPhysScale(origin, dim); }
	int setOctreeLevels(int levels);

	template <typename T>
	int addVariable(std::string name, T *data, std::string usrCompression="", std::string otherParams="");
	template <typename T, typename A>
  	void addVariable(std::string name, std::vector<T, A> &Data, std::string usrCompression="", std::string otherParams="");

	int write();
};

inline void IO::writeLogToDisk(std::string logFilename, std::string logContents)
{
	int myRank;
	MPI_Comm_rank(myComm, &myRank);

	std::ofstream myfile;
  	myfile.open( (logFilename + std::to_string(myRank)).c_str());
  	myfile << logContents;
  	myfile.close();
}


inline IO::IO(std::string _filetype, std::string filename, MPI_Comm comm)
{
	octreeLevels = 0; 
	filetype = "";
	prototypeIO = false;
	myComm = comm;

	writer = new gio::GenericIO(comm, filename);
	setFileType(_filetype);
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
	nlohmann::json jsonInput = nlohmann::json::parse(fileTypes);
	nlohmann::json data = jsonInput["hacc-file-types"];
	std::vector<std::string> filetypes;
	for (auto it=data.begin(); it!=data.end(); ++it)
		filetypes.push_back( it.value() );


	// Check to see if that filetype is legit
	auto found = std::find(filetypes.begin(), filetypes.end(), filetype);
	if (found != filetypes.end())
	{
		// pull in the definition of the file
		if (*found == "standard-output")
			fileInfo = nlohmann::json::parse(standard_output);
		else if (*found == "checkpoint")
			fileInfo = nlohmann::json::parse(checkpoint);
		else
		{
			std::cout << "Cannot process that file type" << std::endl;
			return -1;
		}

		
		// Check Octree
		if (fileInfo.contains("octee"))
		{
			if (fileInfo["octee"] == "Never")
				octreeLevels = -1;
			else if (fileInfo["octee"] == "Off")
				octreeLevels = 0;
			else if (fileInfo["octee"].is_number())
			{
				octreeLevels = fileInfo["octee"];
			}
		}

		log << "Octree level: " << octreeLevels << std::endl;



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

		log << "max file compression: " << maxFileCompression << std::endl;


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
	//if (octreeLevels > 0)
	//	writer->useOctree(octreeLevels);

	writer->write();

	return 1;
}


inline std::vector<std::string> IO::splitString(std::string const &str, const char* delim)
{
	std::vector<std::string> out;
	char *token = strtok(const_cast<char*>(str.c_str()), delim);
	while (token != nullptr)
	{
		out.push_back(std::string(token));
		token = strtok(nullptr, delim);
	}

	return out;
}




template <typename T, typename A>
void IO::addVariable(std::string name, std::vector<T, A> &Data, std::string usrCompression, std::string otherParams)
{
   	T *D = Data.empty() ? 0 : &Data[0];
    addVariable(name, D, usrCompression, otherParams);
}


template <typename T>
inline int IO::addVariable(std::string name, T *data, std::string usrCompression, std::string otherParams)
{
	if (prototypeIO)
	{
		//writer->addVariable(name, data, flags, compressParams);
		return 1;
	}

	log << "\n\nAdding variable " << name << std::endl;

	bool scalarFound;
	int scalarIndex;
	std::vector<std::string> params;


	// Check if filetype is valid
	if ( !fileInfo.contains("variables") )
	{
		std::cout << "The specified filetype " << filetype << " does not contain any variables. This could be a bug!" << std::endl;
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

			log << "JSON Params: " << std::endl;
			// Gather the other specs for that variable
			for (int j=1; j<fileInfo["variables"][i].size(); j++)
			{
				params.push_back(fileInfo["variables"][i][j]);
				log << " - " << fileInfo["variables"][i][j] << std::endl; 
			}


			// done
			break;
		}
	}


	// Foiling attempt to add non existing scalar
	if (!scalarFound)
	{
		std::cout << "Varaible " << name << " does not exist for filetype " << filetype << std::endl;
		return -1;
	}
	

	//
	// Process parameters

	// Flags
	unsigned flags = 0;
	if ( fileInfo["variables"][scalarIndex].contains("CoordX") )
		flags = flags | gio::GenericIO::VarIsPhysCoordX;
	else if ( fileInfo["variables"][scalarIndex].contains("CoordY") )
		flags = flags | gio::GenericIO::VarIsPhysCoordY;
	else if ( fileInfo["variables"][scalarIndex].contains("CoordZ") )
		flags = flags | gio::GenericIO::VarIsPhysCoordZ;
		
	if ( fileInfo["variables"][scalarIndex].contains("extra-space") )
		flags = flags | gio::GenericIO::VarHasExtraSpace;

	log << "flags: " << flags << std::endl;


	


	// No Compression for file
	if ( (maxFileCompression == "None") )
	{
		writer->addVariable(name, data, flags);

		log << "\nNo compression" << std::endl;
		writeLogToDisk("log_",log.str());

		return 1;
	}

	
	// User did not specify anythging - find Compression spefified in JSON spec file
	int maxCompressorSpecsLevel = 0;	
	if ( usrCompression == "" )
	{
		log << "\nNo user speficied compression" << std::endl;

		std::string specsCompressor = "";
		std::string maxCompressor = "";
		
		for (int i=0; i<fileInfo["variables"][scalarIndex].size(); i++)
		{
			std::string tempStr = fileInfo["variables"][scalarIndex][i];

			std::string defCompStr = "default-compressor:";
			if (tempStr.compare(0, defCompStr.length(), defCompStr) == 0)
				specsCompressor = tempStr.substr(defCompStr.length(), tempStr.length()-defCompStr.length());

			std::string maxCompStr = "max-compression-level:";
			if (tempStr.compare(0, maxCompStr.length(), maxCompStr) == 0)
				maxCompressor = tempStr.substr(maxCompStr.length(), tempStr.length()-maxCompStr.length());
		}
		log << "specsCompressor: " << specsCompressor << std::endl;
		log << "maxCompressorSpecs: " << maxCompressor << std::endl;

		
		
		if (maxCompressor == "Lossy" || maxCompressor == "")
			maxCompressorSpecsLevel = 2;
		else if (maxCompressor == "Lossless")
			maxCompressorSpecsLevel = 1;
		else // None
			maxCompressorSpecsLevel = 0;

		log << "maxCompressorSpecsLevel: " << maxCompressorSpecsLevel << std::endl;

	

		// Find the compressor to use
		std::string compressorName = "";
		std::size_t found_pos = specsCompressor.find("~");
  		if (found_pos != std::string::npos)
  			compressorName = specsCompressor.substr(0,found_pos);
  		else
    		compressorName = specsCompressor.substr(0,specsCompressor.length());

    	log << "\nCompressor name: " << compressorName << std::endl;
    	


		// Find the parameters to use for the compressor
    	std::string compParams = "";
    	found_pos = specsCompressor.find(":");
  		if (found_pos != std::string::npos)
  			compParams = specsCompressor.substr(found_pos+1,specsCompressor.length());

  		const char* delim = " ";
		std::vector<std::string> userCompressParams = splitString(compParams,delim);
		for (int i=0; i<userCompressParams.size(); i++)
			log << " - jsonCompressParams " << i << ": " << userCompressParams[i] << std::endl;



		// Specify compression for GenericIO
		//writer->addVariable(name, data, flags);

	}


	if ( usrCompression != "" ) //Get user specified compression
	{
		log << "\nUser speficied compression" << std::endl;

		std::string usrCompStr = "compress:";
		std::string usrComp = usrCompression.substr(usrCompStr.length(), usrCompression.length()-usrCompStr.length());


		// Find the compressor to use
		std::string compressorName = "";
		std::size_t found_pos = usrComp.find("~");
  		if (found_pos != std::string::npos)
  			compressorName = usrComp.substr(0, found_pos);
  		else
    		compressorName = usrComp.substr(0, usrComp.length());
    	
    	log << "Compressor name: " << compressorName << std::endl;



		// Find the parameters to use for the compressor
    	std::string compParams = "";
    	found_pos = usrComp.find(":");
  		if (found_pos != std::string::npos)
  			compParams = usrComp.substr(found_pos+1,usrComp.length());

    	const char* delim = " ";
		std::vector<std::string> userCompressParams = splitString(compParams,delim);
		for (int i=0; i<userCompressParams.size(); i++)
			log << " - userCompressParams " << i << ": " << userCompressParams[i] << std::endl;


		// Specify compression for GenericIO
		if (userCompressParams[0] == "None")
		{
		  	//writer->addVariable(name, data, flags);
		}
		else
			if (userCompressParams[0] == "SZ" && maxCompressorSpecsLevel == 2)	// User wants lossy and specs ok with that
			{
		 		//writer->addVariable(name, data, flags, userCompressorSpecs);
			}
			else
		 		if (userCompressParams[0] == "BLOSC" && maxCompressorSpecsLevel >= 1)	// User wants lossless and specs ok with that
		 		{
		 			//writer->addVariable(name, data, flags, userCompressorSpecs);
		 		}
		 		else // disagreement, do not compress
		 		{ 
		 			//writer->addVariable(name, data, flags);
		 		}
	}

	writeLogToDisk("log_",log.str());

	return 1;
	
}



} // namespace IO_Layer