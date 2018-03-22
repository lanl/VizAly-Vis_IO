#!/bin/bash
BLOSC_O="/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/blosc.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/blosclz.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/shuffle.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/bitshuffle-generic.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/shuffle-generic.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzwrite.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/crc32.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inffast.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/zutil.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/infback.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/deflate.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inflate.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzread.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzlib.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzclose.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/uncompr.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/compress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inftrees.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/trees.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/adler32.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/lz4-1.7.2/lz4.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/lz4-1.7.2/lz4hc.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v01.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v02.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v03.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v06.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v04.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v05.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/fse_compress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/zstd_compress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/huf_compress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/zbuff_compress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/entropy_common.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/xxhash.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/zstd_common.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/fse_decompress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/dictBuilder/zdict.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/dictBuilder/divsufsort.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/zstd_decompress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/huf_decompress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/zbuff_decompress.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-c.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-sinksource.o \
	/home/pascal/projects/VizAly/genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-stubs-internal.o"

echo $BLOSC_O

mpicxx fileReading.cpp -fopenmp -I/home/pascal/projects/VizAly/genericio \
	/home/pascal/projects/VizAly/genericio/mpi/GenericIO.o $BLOSC_O \
	-o fileReading


mpicxx dataGen.cpp -fopenmp -I/home/pascal/projects/VizAly/genericio \
	/home/pascal/projects/VizAly/genericio/mpi/GenericIO.o $BLOSC_O \
	-o dataGen