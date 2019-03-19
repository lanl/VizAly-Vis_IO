/*
 *                    Copyright (C) 2015, UChicago Argonne, LLC
 *                               All Rights Reserved
 *
 *                               Generic IO (ANL-15-066)
 *                 Pascal Grosset, Los Alamos National Laboratory
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
           

            
            // Get all the other info
            GIO.openAndReadHeader(gio::GenericIO::MismatchAllowed, -1, true);

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
                std::cout <<  i << ": " << VI[i].Name << ", " << VI[i].Size << ", " <<  VI[i].IsFloat << ", " <<  VI[i].IsSigned << ", " << VI[i].MaybePhysGhost;

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



            if (GIO.isOctree())
            {
                GIO.printOctree();

                int NR = GIO.readNRanks();  // num ranks

                GIOOctree octreeData = GIO.getOctree();

                for (int r = 0; r < NR; ++r)
                {
                    size_t NElem = GIO.readNumElems(r);

                    std::vector<float> xx, yy, zz;
                    xx.resize(NElem);
                    yy.resize(NElem);
                    zz.resize(NElem);

                    GIO.addVariable( "x", &xx[0], true);
                    GIO.addVariable( "y", &yy[0], true);
                    GIO.addVariable( "z", &zz[0], true);

                    GIO.readDataSection(0, NElem, r, false); 

                    std::cout << "\n\nRank: " << r << ", #elements: " << NElem << std::endl;

                    for (int l=0; l<octreeData.rows.size(); l++)
                    {
                        size_t octreeRankOffset, octreeRankNumRows;
                        if ( octreeData.rows[l].partitionLocation ==  r)    // Find ranks that i am reading now 
                        {
                            octreeRankOffset  = octreeData.rows[l].offsetInFile;
                            octreeRankNumRows = octreeData.rows[l].numParticles;

                            std::cout << "Rows for octree leaf " << l << " in rank " << r << " with extents " <<
                                octreeData.rows[l].minX << "-" << octreeData.rows[l].maxX << ", " <<
                                octreeData.rows[l].minY << "-" << octreeData.rows[l].maxY << ", " <<
                                octreeData.rows[l].minZ << "-" << octreeData.rows[l].maxZ;

                            
                            bool outside = false;
                            for (size_t j = octreeRankOffset; j < octreeRankOffset+octreeRankNumRows; ++j)
                            {
                                float x = xx[j];
                                float y = yy[j];
                                float z = zz[j];

                                //std::cout << x << ", " << y << ", " << z << std::endl;

                                if (x > octreeData.rows[l].maxX || x < octreeData.rows[l].minX)
                                {
                                    outside = true;
                                    std::cout << "Out of bounds for " << x << ", " << y << ", " << z << std::endl;
                                }

                                if (y > octreeData.rows[l].maxY || y < octreeData.rows[l].minY)
                                {
                                    outside = true;
                                    std::cout << "Out of bounds for " << x << ", " << y << ", " << z << std::endl;
                                }

                                if (z > octreeData.rows[l].maxZ || z < octreeData.rows[l].minZ)
                                {
                                    outside = true;
                                    std::cout << "Out of bounds for " << x << ", " << y << ", " << z << std::endl;
                                }
                            }

                            if (outside == false)
                                std::cout << " | All particles in bounds for leaf " << l << " in rank " << r << std::endl ;
                            
                        }
                    }
                }
            }


            GIO.clearVariables();
            GIO.close();
        }
    }
    #endif

    return 0;
}

// e.g. run:
// $ frontend/GenericIOFileInfo m000.full.mpicosmo.499
