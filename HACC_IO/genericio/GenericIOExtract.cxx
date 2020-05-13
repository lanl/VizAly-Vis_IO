#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <stdint.h>
#include <fstream>

#include "GenericIO.h"

using namespace gio;
using namespace std;


inline std::string determineDataType(int size, bool isFloat, bool isSigned)
{
    std::string dataType;

    if (isFloat)
    {
        if (size == 4)
            dataType = "float";
        else if (size == 8)
            dataType = "double";
        else
            return "";
    }
    else // not float
    {
        if (isSigned)
        {
            if (size == 1)
                dataType = "int8_t";
            else if (size == 2)
                dataType = "int16_t";
            else if (size == 4)
                dataType = "int32_t";
            else if (size == 8)
                dataType = "int64_t";
            else
                return "";
        }
        else
        {
            if (size == 1)
                dataType = "uint8_t";
            else if (size == 2)
                dataType = "uint16_t";
            else if (size == 4)
                dataType = "uint32_t";
            else if (size == 8)
                dataType = "uint64_t";
            else
                return "";
        }
    }

    return dataType;
}


inline int allocateMem(std::string dataType, size_t numElements, int offset, void *& data)
{
    if (dataType == "float")
        data = new float[numElements + offset];
    else if (dataType == "double")
        data = new double[numElements + offset];
    else if (dataType == "int")
        data = new int[numElements + offset];
    else if (dataType == "int8_t")
        data = new int8_t[numElements + offset];
    else if (dataType == "int16_t")
        data = new int16_t[numElements + offset];
    else if (dataType == "int32_t")
        data = new int32_t[numElements + offset];
    else if (dataType == "int64_t")
        data = new int64_t[numElements + offset];
    else if (dataType == "uint8_t")
        data = new uint8_t[numElements + offset];
    else if (dataType == "uint16_t")
        data = new uint16_t[numElements + offset];
    else if (dataType == "uint32_t")
        data = new uint32_t[numElements + offset];
    else if (dataType == "uint64_t")
        data = new uint64_t[numElements + offset];
    else
        return 0;

    return 1;
}


inline int deAllocateMem(std::string dataType, void *& data)
{
    if (data == NULL) // already deallocated!
        return 1;

    if (dataType == "float")
        delete[](float*) data;
    else if (dataType == "double")
        delete[](double*) data;
    else if (dataType == "int")
        delete[](int*) data;
    else if (dataType == "int8_t")
        delete[](int8_t*) data;
    else if (dataType == "int16_t")
        delete[](int16_t*) data;
    else if (dataType == "int32_t")
        delete[](int32_t*) data;
    else if (dataType == "int64_t")
        delete[](int64_t*) data;
    else if (dataType == "uint8_t")
        delete[](uint8_t*) data;
    else if (dataType == "uint16_t")
        delete[](uint16_t*) data;
    else if (dataType == "uint32_t")
        delete[](uint32_t*) data;
    else if (dataType == "uint64_t")
        delete[](uint64_t*) data;
    else
        return 0;

    data = NULL;

    return 1;
}



int main(int argc, char *argv[])
{
    
  #ifndef GENERICIO_NO_MPI
    MPI_Init(&argc, &argv);
  #endif

    if (argc < 4)
    {
        cerr << "Usage: " << argv[0] << " <mpiioOld> <mpiioNew> <rank> <scalar>" << endl;
        exit(-1);
    }


    int Rank, NRanks;
  #ifndef GENERICIO_NO_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
    MPI_Comm_size(MPI_COMM_WORLD, &NRanks);
  #else
    Rank = 0;
    NRanks = 1;
  #endif

    int arg = 1;
    string FileName(argv[arg++]);


    {
        unsigned Method = GenericIO::FileIOPOSIX;
      #ifndef GENERICIO_NO_MPI
        const char *EnvStr = getenv("GENERICIO_USE_MPIIO");
        if (EnvStr && string(EnvStr) == "1")
            Method = GenericIO::FileIOMPI;
      #endif


      #ifndef GENERICIO_NO_MPI
        GenericIO GIO(MPI_COMM_SELF, FileName, Method);
      #else
        GenericIO GIO(FileName, Method);
      #endif
        GIO.openAndReadHeader(GenericIO::MismatchAllowed, -1);

        
        int NR = GIO.readNRanks();
        
        vector<GenericIO::VariableInfo> VI;
        GIO.getVariableInfo(VI);

        size_t NElem = 0;
        for (int i=0; i<NR; i++)
            NElem += GIO.readNumElems(i);


        double PhysOrigin[3], PhysScale[3];
        GIO.readPhysOrigin(PhysOrigin);
        GIO.readPhysScale(PhysScale);

        int Dims[3];
        GIO.readDims(Dims);
       

        // Some output
        if (Rank == 0)
        {
            std::cout << "num ranks: " << NR << std::endl;
            std::cout << "total elements: " << NElem << std::endl;
    		std::cout << "Dims: " << Dims[0] << ", " << Dims[1] << ", " << Dims[2] << std::endl;
    		std::cout << "PhysOrigin: " << PhysOrigin[0] << ", " << PhysOrigin[1] << ", " << PhysOrigin[2] << std::endl;
    		std::cout << "readPhysScale: " << PhysScale[0] << ", " << PhysScale[1] << ", " << PhysScale[2] << std::endl;
        }



        int dumpRank = atoi(argv[3]);
        std::vector<std::string> scalars;
        for (int i=4; i<argc; i++)
            scalars.push_back( argv[i] );


        FILE * pFile;
        pFile = fopen(argv[2], "wb");

        //GIO.readData(dumpRank, false);
        size_t elemsForRank = GIO.readNumElems(dumpRank);

        size_t file_offset = 0;
        for (size_t i = 0; i < VI.size(); ++i)
        {
            for (size_t s = 0; s < scalars.size(); ++s)
                if (VI[i].Name == scalars[s])
                {
                    void *data;
                    int dataSize = static_cast<int>(VI[i].Size);
                    std::string dataType = determineDataType(dataSize, VI[i].IsFloat, VI[i].IsSigned);

                    std::cout << "Scalar " << scalars[s] << " found; num elements: " << elemsForRank << " - " << dataType << " data size: " << dataSize << std::endl;

                    size_t offset = 0;
                   
                    allocateMem(dataType, elemsForRank, offset, data);



                    if (dataType == "float")
                        GIO.addVariable( (VI[i].Name).c_str(), (float*)data, true);
                    else if (dataType == "double")
                        GIO.addVariable( (VI[i].Name).c_str(), (double*)data, true);
                    else if (dataType == "int8_t")
                        GIO.addVariable((VI[i].Name).c_str(), (int8_t*)data, true);
                    else if (dataType == "int16_t")
                        GIO.addVariable((VI[i].Name).c_str(), (int16_t*)data, true);
                    else if (dataType == "int32_t")
                        GIO.addVariable((VI[i].Name).c_str(), (int32_t*)data, true);
                    else if (dataType == "int64_t")
                        GIO.addVariable((VI[i].Name).c_str(), (int64_t*)data, true);
                    else if (dataType == "uint8_t")
                        GIO.addVariable((VI[i].Name).c_str(), (uint8_t*)data, true);
                    else if (dataType == "uint16_t")
                        GIO.addVariable((VI[i].Name).c_str(), (uint16_t*)data, true);
                    else if (dataType == "uint32_t")
                        GIO.addVariable((VI[i].Name).c_str(), (uint32_t*)data, true);
                    else if (dataType == "uint64_t")
                        GIO.addVariable((VI[i].Name).c_str(), (uint64_t*)data, true);
                
                    GIO.readData(dumpRank, false); // reading the whole file

                    fseek (pFile, file_offset*4, SEEK_SET);
                    fwrite(data, static_cast<int>(VI[i].Size), elemsForRank, pFile);  

                    file_offset = file_offset + elemsForRank;
                    break;
                }        
        }

        fclose(pFile);


        
    }
   

  #ifndef GENERICIO_NO_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
  #endif

    return 0;
}

// Run as:
// $ frontend/GenericIOExtract <input_file_name> <output_file_name> rank <scalar1> <scalar2> ...
// $ frontend/GenericIOExtract ../../data/Argonne/STEP499/m000.full.mpicosmo.499 testOutput_255.raw 255 x y z
// num ranks: 256
// total elements: 1073726359
// Dims: 8, 8, 4
// PhysOrigin: 0, 0, 0
// readPhysScale: 256, 256, 256
// Scalar x found; num elements: 5991271 - float data size: 4
// Scalar y found; num elements: 5991271 - float data size: 4
// Scalar z found; num elements: 5991271 - float data size: 4

