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

#include "gio.h"
#include <iostream>

void read_gio_float(char* file_name, char* var_name, float* data, int field_count, int rank)
{
	read_gio<float>(file_name, var_name, data, field_count, rank);
}

void read_gio_double(char* file_name, char* var_name, double* data, int field_count, int rank)
{
	read_gio<double>(file_name, var_name, data, field_count, rank);
}

void read_gio_uint16(char* file_name, char* var_name, uint16_t* data, int field_count, int rank)
{
	read_gio<uint16_t>(file_name, var_name, data, field_count, rank);
}

void read_gio_int32(char* file_name, char* var_name, int* data, int field_count, int rank)
{
	read_gio<int>(file_name, var_name, data, field_count, rank);
}

void read_gio_int64(char* file_name, char* var_name, int64_t* data, int field_count, int rank)
{
	read_gio<int64_t>(file_name, var_name, data, field_count, rank);
}




// Get the number of scalars in that file
int64_t get_num_scalars(char* file_name)
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	std::vector<gio::GenericIO::VariableInfo> VI;
	reader.getVariableInfo(VI);

	return VI.size();
}


// Return the name from index
char* get_scalar_name(char* file_name, int scalarIndex)
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	std::vector<gio::GenericIO::VariableInfo> VI;
	reader.getVariableInfo(VI);

	std::string scaler_name = VI[scalarIndex].Name;
	char *temp_name = new char[scaler_name.size() + 1];
	strcpy(temp_name, scaler_name.c_str());

	return temp_name;
}




// get data type of the scalar
var_type get_scalar_type(char* file_name, char* scalar_name)
{
	gio::GenericIO reader(file_name);
	std::vector<gio::GenericIO::VariableInfo> VI;
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
	reader.getVariableInfo(VI);

	int num = VI.size();
	for (int i = 0; i < num; ++i)
	{
		gio::GenericIO::VariableInfo vinfo = VI[i];
		if (vinfo.Name == scalar_name)
		{
			if (vinfo.IsFloat && vinfo.ElementSize == 4)
				return float_type;
			else if (vinfo.IsFloat && vinfo.ElementSize == 8)
				return double_type;
			else if (!vinfo.IsFloat && vinfo.ElementSize == 2)
				return uint16_type;
			else if (!vinfo.IsFloat && vinfo.ElementSize == 4)
				return int32_type;
			else if (!vinfo.IsFloat && vinfo.ElementSize == 8)
				return int64_type;
			else
				return type_not_found;
		}
	}
	return var_not_found;
}


// TODO: Don't remember anymore
int get_scalar_field_count(char* file_name, char* scalar_name)
{
	gio::GenericIO reader(file_name);
	std::vector<gio::GenericIO::VariableInfo> VI;
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
	reader.getVariableInfo(VI);

	int num = VI.size();
	for (int i = 0; i < num; ++i)
	{
		gio::GenericIO::VariableInfo vinfo = VI[i];
		if (vinfo.Name == scalar_name)
		{
			return vinfo.Size / vinfo.ElementSize;
		}
	}
	return 0;
}




// Return number of entries in a file
int64_t get_elem_num(char* file_name, int rank)
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
	int num_ranks = reader.readNRanks();

	uint64_t size = 0;

	if (rank == -1)
		for (int i = 0; i < num_ranks; ++i)
			size += reader.readNumElems(i);
	else
		size += reader.readNumElems(rank);


	reader.close();
	return size;
}




// Get number of ranks in that file
int get_num_ranks(char* file_name)
{
	int num_ranks = 0;
	{
		gio::GenericIO reader(file_name);
		reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
		num_ranks = reader.readNRanks();
	}

	return num_ranks;
}


// Get number of ranks in that extent for a file
int get_num_ranks_in(char* file_name, int extents[])
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	int dims[3];
	reader.readDims(dims);

	double physOrigin[3], physScale[3];
	reader.readPhysOrigin(physOrigin);
	reader.readPhysScale(physScale);

	std::vector<int> intersectedRanks;

	for (int r = 0; r < reader.readNRanks(); ++r)
	{
		int coords[3];
		reader.readCoords(coords, r);


		int currentExtents[6];

		// x
		currentExtents[0] = physOrigin[0] + coords[0] * (physScale[0] / dims[0]);
		currentExtents[1] = currentExtents[0] + (physScale[0] / dims[0]);

		// y
		currentExtents[2] = physOrigin[1] + coords[1] * (physScale[1] / dims[1]);
		currentExtents[3] = currentExtents[2] + (physScale[1] / dims[1]);

		// z
		currentExtents[4] = physOrigin[2] + coords[2] * (physScale[2] / dims[2]);
		currentExtents[5] = currentExtents[4] + (physScale[2] / dims[2]);

		if (intersect(extents, currentExtents))
			intersectedRanks.push_back(r);
	}

	return intersectedRanks.size();
}


// Get the ranks in that extent for a file
int* get_ranks_in(char* file_name, int extents[])
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	int dims[3];
	reader.readDims(dims);

	double physOrigin[3], physScale[3];
	reader.readPhysOrigin(physOrigin);
	reader.readPhysScale(physScale);

	std::vector<int> intersectedRanks;

	for (int r = 0; r < reader.readNRanks(); ++r)
	{
		int coords[3];
		reader.readCoords(coords, r);


		int currentExtents[6];

		// x
		currentExtents[0] = physOrigin[0] + coords[0] * (physScale[0] / dims[0]);
		currentExtents[1] = currentExtents[0] + (physScale[0] / dims[0]);

		// y
		currentExtents[2] = physOrigin[1] + coords[1] * (physScale[1] / dims[1]);
		currentExtents[3] = currentExtents[2] + (physScale[1] / dims[1]);

		// z
		currentExtents[4] = physOrigin[2] + coords[2] * (physScale[2] / dims[2]);
		currentExtents[5] = currentExtents[4] + (physScale[2] / dims[2]);

		if (intersect(extents, currentExtents))
			intersectedRanks.push_back(r);
	}

	int *x = new int[intersectedRanks.size()];
	std::copy(intersectedRanks.begin(), intersectedRanks.end(), x);

	return x;
}




extern "C" void inspect_gio(char* file_name)
{
	int64_t size = get_elem_num(file_name);
	gio::GenericIO reader(file_name);
	std::vector<gio::GenericIO::VariableInfo> VI;
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
	reader.getVariableInfo(VI);
	std::cout << "Number of Elements: " << size << std::endl;
	int num = VI.size();
	std::cout << "[data type] Variable name" << std::endl;
	std::cout << "---------------------------------------------" << std::endl;
	for (int i = 0; i < num; ++i)
	{
		gio::GenericIO::VariableInfo vinfo = VI[i];

		if (vinfo.IsFloat)
			std::cout << "[f";
		else
			std::cout << "[i";
		int NumElements = vinfo.Size / vinfo.ElementSize;
		std::cout << " " << vinfo.ElementSize * 8;
		if (NumElements > 1)
			std::cout << "x" << NumElements;
		std::cout << "] ";
		std::cout << vinfo.Name << std::endl;
	}
	std::cout << "\n(i=integer,f=floating point, number bits size)" << std::endl;

	if (reader.isOctree())
	{
		std::cout << "---------------------------------------------" << std::endl;
		std::cout << "Octree info:" << std::endl;
		reader.printOctree();
	}
}



//
// Octree
//

void read_gio_oct_float(char* file_name, int leaf_id, char* var_name, float* data)
{
	read_gio_rankLeaf<float>(file_name, leaf_id, var_name, data);
}
void read_gio_oct_double(char* file_name, int leaf_id, char* var_name, double* data)
{
	read_gio_rankLeaf<double>(file_name, leaf_id, var_name, data);
}

void read_gio_oct_int16(char* file_name, int leaf_id, char* var_name, uint16_t* data)
{
	read_gio_rankLeaf<uint16_t>(file_name, leaf_id, var_name, data);
}

void read_gio_oct_int32(char* file_name, int leaf_id, char* var_name, int* data)
{
	read_gio_rankLeaf<int>(file_name, leaf_id, var_name, data);
}
void read_gio_oct_int64(char* file_name, int leaf_id, char* var_name, int64_t* data)
{
	read_gio_rankLeaf<int64_t>(file_name, leaf_id, var_name, data);
}


char* get_octree(char* file_name)
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	std::string octreeStr;
	if (reader.isOctree())
		octreeStr = (reader.getOctree()).getOctreeStr();


	char *temp_name = new char[octreeStr.size() + 1];
	strcpy(temp_name, octreeStr.c_str());


	return temp_name;
}


int * get_octree_leaves(char* file_name, int extents[])
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	std::vector<int> intersectedLeaves;
	GIOOctree tempOctree = reader.getOctree();
	for (int i = 0; i < tempOctree.rows.size(); i++)
		if ( tempOctree.rows[i].intersect(extents) )
			intersectedLeaves.push_back(i);

	int *x = new int[intersectedLeaves.size()];
	std::copy(intersectedLeaves.begin(), intersectedLeaves.end(), x);

	return x;
}


int get_num_octree_leaves(char* file_name, int extents[])
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	std::vector<int> intersectedLeaves;
	GIOOctree tempOctree = reader.getOctree();
	for (int i = 0; i < tempOctree.rows.size(); i++)
		if ( tempOctree.rows[i].intersect(extents) )
			intersectedLeaves.push_back(i);

	return intersectedLeaves.size();
}


int64_t get_elem_num_in_leaf(char* file_name, int leaf_id)
{
	gio::GenericIO reader(file_name);
	reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);

	GIOOctree tempOctree = reader.getOctree();
	reader.close();

	return tempOctree.getCount(leaf_id);
}