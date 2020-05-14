#!/bin/bash
PROJECT_HOME=$(pwd)


OPENMPFLAG="-fopenmp"
OPENMPFLAG="" # On OSX

BLOSC_O="genericio/mpi/thirdparty/blosc/blosc.o \
	genericio/mpi/thirdparty/blosc/blosclz.o \
	genericio/mpi/thirdparty/blosc/shuffle.o \
	genericio/mpi/thirdparty/blosc/bitshuffle-generic.o \
	genericio/mpi/thirdparty/blosc/shuffle-generic.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzwrite.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/crc32.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inffast.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/zutil.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/infback.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/deflate.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inflate.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzread.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzlib.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/gzclose.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/uncompr.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/compress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/inftrees.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/trees.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zlib-1.2.8/adler32.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/lz4-1.7.2/lz4.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/lz4-1.7.2/lz4hc.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v01.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v02.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v03.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v06.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v04.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/legacy/zstd_v05.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/fse_compress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/zstd_compress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/huf_compress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/compress/zbuff_compress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/entropy_common.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/xxhash.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/zstd_common.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/common/fse_decompress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/dictBuilder/zdict.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/dictBuilder/divsufsort.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/zstd_decompress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/huf_decompress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/zstd-0.7.4/decompress/zbuff_decompress.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-c.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-sinksource.o \
	genericio/mpi/thirdparty/blosc/internal-complibs/snappy-1.1.1/snappy-stubs-internal.o"
	
SZ_O="genericio/mpi/thirdparty/SZ/sz/src/ByteToolkit.o \
	genericio/mpi/thirdparty/SZ/sz/src/dataCompression.o \
	genericio/mpi/thirdparty/SZ/sz/src/DynamicIntArray.o \
	genericio/mpi/thirdparty/SZ/sz/src/iniparser.o \
	genericio/mpi/thirdparty/SZ/sz/src/CompressElement.o \
	genericio/mpi/thirdparty/SZ/sz/src/DynamicByteArray.o \
	genericio/mpi/thirdparty/SZ/sz/src/rw.o \
	genericio/mpi/thirdparty/SZ/sz/src/TightDataPointStorageI.o \
	genericio/mpi/thirdparty/SZ/sz/src/TightDataPointStorageD.o \
	genericio/mpi/thirdparty/SZ/sz/src/TightDataPointStorageF.o \
	genericio/mpi/thirdparty/SZ/sz/src/conf.o \
	genericio/mpi/thirdparty/SZ/sz/src/DynamicDoubleArray.o \
	genericio/mpi/thirdparty/SZ/sz/src/TypeManager.o \
	genericio/mpi/thirdparty/SZ/sz/src/dictionary.o \
	genericio/mpi/thirdparty/SZ/sz/src/DynamicFloatArray.o \
	genericio/mpi/thirdparty/SZ/sz/src/VarSet.o \
	genericio/mpi/thirdparty/SZ/sz/src/callZlib.o \
	genericio/mpi/thirdparty/SZ/sz/src/Huffman.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_float.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_double.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_int8.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_int16.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_int32.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_int64.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_uint8.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_uint16.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_uint32.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_uint64.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_uint8.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_uint16.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_uint32.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_uint64.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_float.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_double.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_int8.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_int16.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_int32.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_int64.o \
	genericio/mpi/thirdparty/SZ/sz/src/utility.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_float_pwr.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_double_pwr.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_float_pwr.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_double_pwr.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_double_ts.o \
	genericio/mpi/thirdparty/SZ/sz/src/sz_float_ts.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_double_ts.o \
	genericio/mpi/thirdparty/SZ/sz/src/szd_float_ts.o"


mpicxx dataGen.cpp $OPENMPFLAG -std=c++11 -Igenericio \
	$PROJECT_HOME/genericio/mpi/GenericIO.o $BLOSC_O $SZ_O \
	-o dataGen