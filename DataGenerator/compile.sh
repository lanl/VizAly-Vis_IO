#!/bin/bash
PROJECT_HOME=/home/pascal/projects/VizAly

BLOSC_O="$PROJECT_HOME/genericio/mpi/thirdparty/blosc/blosc.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/blosclz.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/shuffle.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/bitshuffle-generic.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/shuffle-generic.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzwrite.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/crc32.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inffast.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/zutil.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/infback.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/deflate.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inflate.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzread.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzlib.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzclose.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/uncompr.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/compress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inftrees.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/trees.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/adler32.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/lz4-1.7.2/lz4.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/lz4-1.7.2/lz4hc.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v01.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v02.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v03.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v06.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v04.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v05.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/fse_compress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/zstd_compress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/huf_compress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/zbuff_compress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/entropy_common.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/xxhash.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/zstd_common.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/fse_decompress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/dictBuilder/zdict.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/dictBuilder/divsufsort.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/zstd_decompress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/huf_decompress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/zbuff_decompress.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-c.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-sinksource.o \
	$PROJECT_HOME/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-stubs-internal.o"

echo $BLOSC_O:

mpicxx fileReading.cpp -fopenmp -I$PROJECT_HOME/genericio \
	$PROJECT_HOME/genericio/mpi/GenericIO.o $BLOSC_O \
	-o fileReading


mpicxx dataGen.cpp -fopenmp -I$PROJECT_HOME/genericio \
	$PROJECT_HOME/genericio/mpi/GenericIO.o $BLOSC_O \
	-o dataGen

mpicxx dataGenNoOct.cpp -fopenmp -I$PROJECT_HOME/genericio \
	$PROJECT_HOME/genericio/mpi/GenericIO.o $BLOSC_O \
	-o dataGenNoOct