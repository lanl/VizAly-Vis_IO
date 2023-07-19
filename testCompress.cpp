#include <iostream>
#include <vector>
#include <random>
#include <mpi.h>

#include "streamingShimComp.hpp"
#include "HACCDataLoader.hpp"

int main(int argc, const char* argv[])
{
    int myRank, numRanks, threadSupport;
	MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &threadSupport);
	MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    HACCDataLoader haccLoader;

    int blockSize;

   

    std::string filename = "/home/pascal/data/HACC/hydro/output_sf_l64n256_ce_fire3_cloudy/m000.full.mpicosmo.624";
    std::cout << "file: " << filename << std::endl;
    haccLoader.init(filename, MPI_COMM_WORLD);
    haccLoader.loadData("x");

    MPI_Barrier(MPI_COMM_WORLD);

    std::cout << "!BBBBB" << std::endl;
    std::cout << "# elements: " << haccLoader.numElements << std::endl;
    size_t totalSize = haccLoader.numElements*sizeof(float);


    blockSize = haccLoader.numElements;
     if (argc == 2)
        blockSize = std::stoi(argv[1]);

    std::cout << "Block size: " << blockSize;
    StreamingShimComp cmpStr; 
    //cmpStr.compress(&randomArray[0], "float", sizeof(float), dims, blockSize);


    cmpStr.setParam("compressor","SZ");
    cmpStr.setParam("abs","0.03");
    cmpStr.setParam("type","float");

    // cmpStr.setParam("compressor","BLOSC");
    size_t cmpSize = cmpStr.stmCompress((float*)haccLoader.data, "float", sizeof(float), haccLoader.numElements, blockSize);

    std::cout << ", compression size: " << cmpSize;
    std::cout << ", huffman tree ratio: " << (float)cmpStr.getHuffSize()/cmpSize;
    std::cout << ", compression ratio: " << (float)totalSize/cmpSize << std::endl;

    cmpStr.stmDecompressHeader();
    //cmpStr.stmDecompressData();


    MPI_Barrier(MPI_COMM_WORLD);

    /*
    void *data;

    for (int i=0; i<10; i++)
    {
        
        cmpStr.stmDecompressNextBlock(data);
        std::cout << ((float *)data)[0] << std::endl;

        delete[](float*) data;
    }
    */

    MPI_Finalize();

    return 0;
}

/*
file: /home/pascal/data/HACC/hydro/output_sf_l64n256_ce_fire3_cloudy/m000.full.mpicosmo.624
variable: x

# elements: 33 554 432, # blocks: 1,       compression ratio: 6.94372      (19329364) 
# elements: 33 554 432, # blocks: 2098,    compression ratio: 6.53973      (16000)
# elements: 33 554 432, # blocks: 4195,    compression ratio: 6.21056      (8000)
# elements: 33 554 432, # blocks: 8389,    compression ratio: 5.7213       (4000)
# elements: 33 554 432, # blocks: 16778,   compression ratio: 5.0374       (2000)
# elements: 33 554 432, # blocks: 33555,   compression ratio: 4.13204      (1000)

variable: y
# elements: 33554432, # blocks: 1, compression ratio: 6.97813
# elements: 33554432, # blocks: 2098, compression ratio: 6.53986    (16000)
# elements: 33554432, # blocks: 4195, compression ratio: 6.20911    (8000)
# elements: 33554432, # blocks: 8389, compression ratio: 5.71737    (4000)
# elements: 33554432, # blocks: 16778, compression ratio: 5.03323   (2000)
# elements: 33554432, # blocks: 33555, compression ratio: 4.12865   (1000)





Blosc
variable: x
# elements: 33554432, # blocks: 1, compression ratio: 1.41033
# elements: 33554432, # blocks: 1049, compression ratio: 1.46018
# elements: 33554432, # blocks: 8389, compression ratio: 1.44256
*/