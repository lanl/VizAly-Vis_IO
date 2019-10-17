/*
 *                    Copyright (C) 2015, UChicago Argonne, LLC
 *                               All Rights Reserved
 *
 *                               Generic IO (ANL-15-066)
 *                     Hal Finkel, Argonne National Laboratory
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

#include <GenericIO.h>
#include <utils/octree.hpp>

#include <stdint.h>
#include <sstream>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <string.h>


enum var_type
{
	float_type = 0,
	double_type = 1,
	int32_type = 2,
	int64_type = 3,
	uint16_type = 4,
	type_not_found = 9,
	var_not_found = 10
};


inline bool intersect(int extents1[], int extents2[])
{
	if ((extents1[0] < extents2[1]) && (extents1[1] >= extents2[0]))
		if ((extents1[2] < extents2[3]) && (extents1[3] >= extents2[2]))
			if ((extents1[4] < extents2[5]) && (extents1[5] >= extents2[4]))
				return true;

	return false;
}



extern "C" void read_gio_float (char* file_name, char* var_name, float* data, int field_count, int rank);
extern "C" void read_gio_double(char* file_name, char* var_name, double* data, int field_count, int rank);
extern "C" void read_gio_uint16 (char* file_name, char* var_name, uint16_t* data, int field_count, int rank);
extern "C" void read_gio_int32 (char* file_name, char* var_name, int* data, int field_count, int rank);
extern "C" void read_gio_int64 (char* file_name, char* var_name, int64_t* data, int field_count, int rank);


extern "C" int64_t get_num_scalars(char* file_name);
extern "C" char* get_scalar_name(char* file_name, int var);
extern "C" var_type get_scalar_type(char* file_name, char* var_name);
extern "C" int get_scalar_field_count(char* file_name, char* var_name);		// Don't remember what this is

extern "C" int64_t get_elem_num(char* file_name, int rank=-1);

extern "C" int get_num_ranks(char* file_name);
extern "C" int get_num_ranks_in(char* file_name, int extents[]);
extern "C" int* get_ranks_in(char* file_name, int extents[]);

extern "C" void inspect_gio(char* file_name);


template <class T>
void read_gio(char* file_name, std::string var_name, T*& data, int field_count, int rank)
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
	int num_ranks = reader.readNRanks();
	uint64_t max_size = 0;
	uint64_t rank_size[num_ranks];
	for (int i = 0; i < num_ranks; ++i)
	{
		rank_size[i] = reader.readNumElems(i);
		if (max_size < rank_size[i])
			max_size = rank_size[i];
	}

	T* rank_data = new T[max_size * field_count + reader.requestedExtraSpace() / sizeof(T)];
	int64_t offset = 0;
	reader.addScalarizedVariable(var_name, rank_data, field_count, gio::GenericIO::VarHasExtraSpace);

	if (rank == -1)
	{
		for (int i = 0; i < num_ranks; ++i)
		{
			reader.readData(i, false);
			std::copy(rank_data, rank_data + rank_size[i]*field_count, data + offset);
			offset += rank_size[i] * field_count;
		}
	}
	else
	{
		std::cout << "read_gio ~ rank: " << rank << std::endl;
		if (rank >= num_ranks)
			return;

		reader.readData(rank, false);
		std::copy(rank_data, rank_data + rank_size[rank]*field_count, data + offset);
	}

	delete [] rank_data;
	reader.close();
}




//
// Octree Section
//
extern "C" void read_gio_oct_float (char* file_name, int leaf_id, char* var_name, float* data);
extern "C" void read_gio_oct_double(char* file_name, int leaf_id, char* var_name, double* data);
extern "C" void read_gio_oct_int16 (char* file_name, int leaf_id, char* var_name, uint16_t* data);
extern "C" void read_gio_oct_int32 (char* file_name, int leaf_id, char* var_name, int* data);
extern "C" void read_gio_oct_int64 (char* file_name, int leaf_id, char* var_name, int64_t* data);

extern "C" char* get_octree(char* file_name);
extern "C" int* get_octree_leaves(char* file_name, int extents[]);
extern "C" int get_num_octree_leaves(char* file_name, int extents[]);

extern "C" int64_t get_elem_num_in_leaf(char* file_name,  int leaf_id);

//extern "C" int get_octree_rank(char* file_name, int extents[]);
//extern "C" int get_octree_leaf_in_rank(char* file_name, int extents[]);


template <class T>
void read_gio_rankLeaf(char* file_name, int leaf_id, std::string var_name, T*& data)
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	GIOOctree tempOctree = reader.getOctree();
	int rank = tempOctree.getRank(leaf_id);
	size_t num_particles = tempOctree.getCount(leaf_id);
	size_t offset = tempOctree.getOffset(leaf_id);


	T* rank_data = new T[num_particles + reader.requestedExtraSpace()];
	reader.addVariable(var_name, rank_data, gio::GenericIO::VarHasExtraSpace);

	reader.readDataSection(offset, num_particles, rank, false);
	std::copy(rank_data, rank_data + num_particles, data);

	delete [] rank_data;

	reader.close();
}
