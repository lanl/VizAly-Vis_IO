#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "GenericIO.h"
#include "gioData.h"




int main(int argc, char *argv[])
{

	std::unordered_multiset<std::string> entries;

	{
		std::cout << "reading file  " << argv[1] << std::endl;

		gio::GenericIO *gioReader;
		gioReader = new gio::GenericIO(argv[1], gio::GenericIO::FileIOPOSIX);

		gioReader->openAndReadHeader(gio::GenericIO::MismatchAllowed);
		int numDataRanks = gioReader->readNRanks();


		// Count number of elements
		size_t totalNumberOfElements = 0;
		size_t maxNumElementsPerRank = 0;
		for (int i=0; i<numDataRanks; ++i)
		{
			totalNumberOfElements += gioReader->readNumElems(i);
			maxNumElementsPerRank = std::max(maxNumElementsPerRank, gioReader->readNumElems(i));
		}


		// Read in the scalars information
		std::vector<gio::GenericIO::VariableInfo> VI;
		gioReader->getVariableInfo(VI);
		int numVars = static_cast<int>(VI.size());


		//
		// Read in data
		for (int r=0; r<numDataRanks; ++r)
	    {
	    	std::cout << "processing rank " << r << std::endl;

	        size_t NElem = gioReader->readNumElems(r);


	        std::vector<GioData> readInData;
			for (size_t i=0; i<numVars; i++)
			{
				GioData temp;
				temp.init(i, VI[i].Name, static_cast<int>(VI[i].Size), VI[i].IsFloat, VI[i].IsSigned, VI[i].IsPhysCoordX, VI[i].IsPhysCoordY, VI[i].IsPhysCoordZ);
				temp.determineDataType();

				temp.setNumElements(maxNumElementsPerRank);
				temp.allocateMem(1);


				readInData.push_back(temp);

				if (readInData[i].dataType == "float")
					gioReader->addVariable( (readInData[i].name).c_str(), (float*)readInData[i].data, true);
				else if (readInData[i].dataType == "double")
					gioReader->addVariable( (readInData[i].name).c_str(), (double*)readInData[i].data, true);
				else if (readInData[i].dataType == "int8_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int8_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "int16_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int16_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "int32_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int32_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "int64_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int64_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint8_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint8_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint16_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint16_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint32_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint32_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint64_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint64_t*)readInData[i].data, true);
			}
		
			gioReader->readData(r, false); 

			
			for (size_t j=0; j<NElem; j++)
			{
				std::string str;
				for (int i=0; i<numVars; i++)
					str += readInData[i].getValue(j) + " ";

				entries.insert( str );
			}

		}
	}

	
	// std::cout << "entries.size() " << entries.size() << "\n";
	// for ( auto local_it = entries.begin(); local_it!= entries.end(); ++local_it )
 //  		std::cout << *local_it << "\n";
	// std::cout << "\n";
   


	{
		std::cout << "\nreading file  " << argv[2] << std::endl;

		gio::GenericIO *gioReader;
		gioReader = new gio::GenericIO(argv[2], gio::GenericIO::FileIOPOSIX);

		gioReader->openAndReadHeader(gio::GenericIO::MismatchAllowed);
		int numDataRanks = gioReader->readNRanks();


		// Count number of elements
		size_t totalNumberOfElements = 0;
		size_t maxNumElementsPerRank = 0;
		for (int i=0; i<numDataRanks; ++i)
		{
			totalNumberOfElements += gioReader->readNumElems(i);
			maxNumElementsPerRank = std::max(maxNumElementsPerRank, gioReader->readNumElems(i));
		}


		// Read in the scalars information
		std::vector<gio::GenericIO::VariableInfo> VI;
		gioReader->getVariableInfo(VI);
		int numVars = static_cast<int>(VI.size());


		//
		// Read in data
		for (int r=0; r<numDataRanks; ++r)
	    {
	    	std::cout << "processing rank " << r << std::endl;

	        size_t NElem = gioReader->readNumElems(r);


	        std::vector<GioData> readInData;
			for (size_t i=0; i<numVars; i++)
			{
				GioData temp;
				temp.init(i, VI[i].Name, static_cast<int>(VI[i].Size), VI[i].IsFloat, VI[i].IsSigned, VI[i].IsPhysCoordX, VI[i].IsPhysCoordY, VI[i].IsPhysCoordZ);
				temp.determineDataType();

				temp.setNumElements(maxNumElementsPerRank);
				temp.allocateMem(1);


				readInData.push_back(temp);

				if (readInData[i].dataType == "float")
					gioReader->addVariable( (readInData[i].name).c_str(), (float*)readInData[i].data, true);
				else if (readInData[i].dataType == "double")
					gioReader->addVariable( (readInData[i].name).c_str(), (double*)readInData[i].data, true);
				else if (readInData[i].dataType == "int8_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int8_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "int16_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int16_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "int32_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int32_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "int64_t")
					gioReader->addVariable((readInData[i].name).c_str(), (int64_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint8_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint8_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint16_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint16_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint32_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint32_t*)readInData[i].data, true);
				else if (readInData[i].dataType == "uint64_t")
					gioReader->addVariable((readInData[i].name).c_str(), (uint64_t*)readInData[i].data, true);
			}
		
			gioReader->readData(r, false); 

			
			for (size_t j=0; j<NElem; j++)
			{
				std::string str;
				for (int i=0; i<numVars; i++)
					str += readInData[i].getValue(j) + " ";

				auto it = entries.find( str );
				if ( it == entries.end() )
					std::cout << str << " does not exist " << std::endl;
				else
					entries.erase ( str );
			}
		}
	}




    if ( entries.size() == 0)
    	std::cout << "Files are identical" << std::endl;
    else
    	std::cout << "Files are different" << std::endl;

	return 0;
}

