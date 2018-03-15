/*
 *                    Copyright (C) 2015, UChicago Argonne, LLC
 *                               All Rights Reserved
 *
 *                               Generic IO (ANL-15-066)
 *                  Pascal Grosset, Los Alamos National Library
 *
 *                              OPEN SOURCE LICENSE
 *
 * Under the terms of Contract No. DE-AC02-06CH11357 with UChicago Argonne,
 * LLC, the U.S. Government retains certain rights in this software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the names of UChicago Argonne, LLC or the Department of Energy
 *      nor the names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 *
 * *****************************************************************************
 *
 *                                  DISCLAIMER
 * THE SOFTWARE IS SUPPLIED “AS IS” WITHOUT WARRANTY OF ANY KIND.  NEITHER THE
 * UNTED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR
 * UCHICAGO ARGONNE, LLC, NOR ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY,
 * EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE
 * ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, DATA, APPARATUS,
 * PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE
 * PRIVATELY OWNED RIGHTS.
 *
 * *****************************************************************************
 */

#include <iostream>
#include <vector>
#include <set>

#include "GenericIO.h"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Program should be called: GenericIOFileInfo InputFileName" << std::endl;
        return 0;
    }

    bool showMap = true;    // In here in case someone wants to turn that off later
    std::string inputFileName = std::string(argv[1]);



    #ifndef GENERICIO_NO_MPI
        std::cout << "Not using MPI here!!!" << std::endl;
    #else
    {
        //
        //  Not using MPI
        //
        {
            unsigned method = gio::GenericIO::FileIOPOSIX;
            gio::GenericIO GIO(inputFileName, method);

            int dims[3];
            int coords[3];
            double physOrigin[3], physScale[3];


            // Get partition Info
            std::vector<int32_t> partitionInfo;
            std::set<int32_t> partitionCounting;
            bool mapInfo = false;
            bool hasPartitions = false;     // BlockHeader
            std::vector< gio::GenericIO::VariableInfo > partitionVI;
            if (showMap == true)
            {
                GIO.openAndReadHeader(gio::GenericIO::MismatchAllowed, -1, !showMap);
                GIO.getVariableInfo(partitionVI);

                if (partitionVI.size() > 1)
                    if (partitionVI[1].Name == "$partition")
                        mapInfo = true;
            }

            if (mapInfo)
            {
                size_t maxNumParticlesPerRank = 0;
                int numRanksInInput = GIO.readNRanks();
                for (int i = 0; i < numRanksInInput; ++i)
                    maxNumParticlesPerRank = std::max(maxNumParticlesPerRank, GIO.readNumElems(i));


                //
                // Create space to read data into
                std::vector< std::vector<char> > Vars(partitionVI.size());
                for (size_t i = 0; i < partitionVI.size(); ++i)
                {

                    Vars[i].resize(partitionVI[i].Size * maxNumParticlesPerRank + GIO.requestedExtraSpace());
                    GIO.addVariable(partitionVI[i], &Vars[i][0], true);
                }


                for (int i = 0; i < numRanksInInput; i++)
                {
                    size_t numElements = GIO.readNumElems(i);
                    GIO.readData(i, false);

                    // Info in rank map; mpi_rank, partition, x, y, z - int32_t
                    for (size_t j = 0; j < numElements; j++)
                    {
                        int32_t _partitionInfo = GIO.getValue<int32_t>(1, j);
                        partitionInfo.push_back(_partitionInfo);
                        partitionCounting.insert(_partitionInfo);
                    }
                }
            }



            // Get all the other info
            showMap = false;
            GIO.openAndReadHeader(gio::GenericIO::MismatchAllowed, -1, !showMap);

            GIO.readDims(dims);
            GIO.readPhysOrigin(physOrigin);
            GIO.readPhysScale(physScale);

            int numRanksInInput = GIO.readNRanks();


            //
            // Create space to read in the data
            std::vector< gio::GenericIO::VariableInfo > VI;
            GIO.getVariableInfo(VI);

            //
            // Create space to store in any data from GIO
            std::cout << "\n# Variables: " << VI.size() << std::endl;
            std::cout << "Pos: Name, Size in bytes, Floating point?, Signed?, ghost?, x or y or z variable | ?: 0=no, 1=yes" << std::endl;
            int numVars = VI.size();
            for (int i = 0; i < numVars; i++)
            {
                std::cout <<  i << ": " << VI[i].Name << ", " << VI[i].Size << ", " <<  VI[i].IsFloat << ", " <<  VI[i].IsSigned << ", " << VI[i].MaybePhysGhost; // << ", " << VI[i].HasExtraSpace;
                if (VI[i].IsPhysCoordX)
                    std::cout << ", x variable" << std::endl;
                else if (VI[i].IsPhysCoordY)
                    std::cout << ", y variable" << std::endl;
                else if (VI[i].IsPhysCoordZ)
                    std::cout << ", z variable" << std::endl;
                else
                    std::cout << "" << std::endl;
            }



            std::cout << "\n3D Split: " << dims[0] << ", " << dims[1] << ", " << dims[2] << std::endl;
            std::cout << "# physical coordinates: (" << physOrigin[0] << ", " << physOrigin[1] << ", " << physOrigin[2] <<
                      ") -> (" << physScale[0]  << ", " << physScale[1]  << ", " << physScale[2]  << ") " << std::endl;



            std::cout << "\n# Ranks in file: " << numRanksInInput << std::endl;
            std::cout << "# Partitions: " << (mapInfo ? std::to_string(partitionCounting.size()) : "1") << std::endl;
            uint64_t particlesCount = 0;
            for (int i = 0; i < numRanksInInput; i++)
            {
                GIO.readCoords(coords, i);
                particlesCount += GIO.readNumElems(i);
                std::cout << i <<  " ~ # particles in global rank: "  << GIO.readGlobalRankNumber(i) << ": " << GIO.readNumElems(i)
                          << ", @ region: " << coords[0] << "," << coords[1] << "," << coords[2]
                          << ", in partition: " << (mapInfo ? std::to_string(partitionInfo[i]) : "1")
                          << ", coordinates: (" << (float)coords[0] / dims[0] * physScale[0] + physOrigin[0] << ", "
                          << (float)coords[1] / dims[1] * physScale[1] + physOrigin[1] << ", "
                          << (float)coords[2] / dims[2] * physScale[2] + physOrigin[2] << ") -> ("
                          << (float)(coords[0] + 1) / dims[0] * physScale[0] + physOrigin[0] << ", "
                          << (float)(coords[1] + 1) / dims[1] * physScale[1] + physOrigin[1] << ", "
                          << (float)(coords[2] + 1) / dims[2] * physScale[2] + physOrigin[2] << ")" << std::endl;
            }
            std::cout << "\nTotal # particles: " << particlesCount << std::endl;

            GIO.clearVariables();
            GIO.close();
        }
    }
    #endif

    return 0;
}

// e.g. run:
// $ frontend/GenericIOFileInfo m000.full.mpicosmo.499

