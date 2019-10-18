#                    Copyright (C) 2015, UChicago Argonne, LLC
#                               All Rights Reserved
# 
#                               Generic IO (ANL-15-066)
#                     Hal Finkel, Argonne National Laboratory
# 
#                              OPEN SOURCE LICENSE
# 
# Under the terms of Contract No. DE-AC02-06CH11357 with UChicago Argonne,
# LLC, the U.S. Government retains certain rights in this software.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#   1. Redistributions of source code must retain the above copyright notice,
#      this list of conditions and the following disclaimer.
# 
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
# 
#   3. Neither the names of UChicago Argonne, LLC or the Department of Energy
#      nor the names of its contributors may be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
# 
# *****************************************************************************
# 
#                                  DISCLAIMER
# THE SOFTWARE IS SUPPLIED "AS IS" WITHOUT WARRANTY OF ANY KIND.  NEITHER THE
# UNTED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR
# UCHICAGO ARGONNE, LLC, NOR ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY,
# EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE
# ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, DATA, APPARATUS,
# PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE
# PRIVATELY OWNED RIGHTS.
# 
# *****************************************************************************

from numpy.ctypeslib import ndpointer
#from __future__ import print_function
import numpy as np
import ctypes as ct
import os
import sys
import pandas as pd

# Define where the library is and load it
_path = os.path.dirname(__file__)
libpygio = ct.CDLL(_path + '/../frontend/libpygio.so')




#
# Interface using ctypes
#

#
# Non octree
libpygio.read_gio_float.restype=None
libpygio.read_gio_float.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_float),ct.c_int,ct.c_int]

libpygio.read_gio_double.restype=None
libpygio.read_gio_double.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_double),ct.c_int,ct.c_int]

libpygio.read_gio_uint16.restype=None
libpygio.read_gio_uint16.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_uint16),ct.c_int,ct.c_int]

libpygio.read_gio_int32.restype=None
libpygio.read_gio_int32.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_int),ct.c_int,ct.c_int]

libpygio.read_gio_int64.restype=None
libpygio.read_gio_int64.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_int64),ct.c_int,ct.c_int]



libpygio.get_num_scalars.restype=ct.c_int
libpygio.get_num_scalars.argtypes=[ct.c_char_p]  

libpygio.get_scalar_name.restype=ct.c_char_p
libpygio.get_scalar_name.argtypes=[ct.c_char_p,ct.c_int]  

libpygio.get_scalar_type.restype=ct.c_int
libpygio.get_scalar_type.argtypes=[ct.c_char_p,ct.c_char_p]

libpygio.get_scalar_field_count.restype=ct.c_int
libpygio.get_scalar_field_count.argtypes=[ct.c_char_p,ct.c_char_p]



libpygio.get_elem_num.restype=ct.c_int64
libpygio.get_elem_num.argtypes=[ct.c_char_p,ct.c_int]



libpygio.get_num_ranks.restype=ct.c_int
libpygio.get_num_ranks.argtypes=[ct.c_char_p]

libpygio.get_num_ranks_in.restype=ct.c_int
libpygio.get_num_ranks_in.argtypes=[ct.c_char_p, ct.POINTER(ct.c_int)]

libpygio.get_ranks_in.restype=ct.POINTER(ct.c_int)
libpygio.get_ranks_in.argtypes=[ct.c_char_p, ct.POINTER(ct.c_int)]



libpygio.inspect_gio.restype=None
libpygio.inspect_gio.argtypes=[ct.c_char_p]



#
# Octree stuff
libpygio.read_gio_oct_int16.restype=None
libpygio.read_gio_oct_int16.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_int)]

libpygio.read_gio_oct_int32.restype=None
libpygio.read_gio_oct_int32.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_int)]

libpygio.read_gio_oct_int64.restype=None
libpygio.read_gio_oct_int64.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_int64)]

libpygio.read_gio_oct_float.restype=None
libpygio.read_gio_oct_float.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_float)]

libpygio.read_gio_oct_double.restype=None
libpygio.read_gio_oct_double.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_double)]



libpygio.get_octree.restype=ct.c_char_p
libpygio.get_octree.argtypes=[ct.c_char_p]

libpygio.get_octree_leaves.restype=ct.POINTER(ct.c_int)
libpygio.get_octree_leaves.argtypes=[ct.c_char_p, ct.POINTER(ct.c_int)]

libpygio.get_num_octree_leaves.restype=ct.c_int
libpygio.get_num_octree_leaves.argtypes=[ct.c_char_p, ct.POINTER(ct.c_int)]


libpygio.get_elem_num_in_leaf.restype=ct.c_int64
libpygio.get_elem_num_in_leaf.argtypes=[ct.c_char_p, ct.c_int]



#
# Additioanl code
#


def read(file_name, var_names, rank_id=-1):
    # Generic read function, read scalars from a file; one rank or full file
    ret = []
    if not isinstance(var_names,list):
        var_names = [ var_names ]

    for var_name in var_names:
        ret.append( read_scalar(file_name, var_name, rank_id) )

    return np.array( ret )


def read_scalar(file_name, var_name, rank):
    var_size = get_num_elements(file_name, rank)

    # Read a scalar from a file at a specific rank or full file
    if sys.version_info[0] == 3:
        file_name = file_name.encode('ascii')
        var_name = var_name.encode('ascii')

    var_type = libpygio.get_scalar_type(file_name,var_name)
    field_count = libpygio.get_scalar_field_count(file_name,var_name)


    if (var_type==10):
        print("scalar not found")
        return
    elif (var_type==9):
        print("scalar type not known (not uint16/int32/int64/float/double)")
    elif (var_type==0):
        #float
        result = np.ndarray((var_size),dtype=np.float32)
        libpygio.read_gio_float(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_float)),field_count,rank)
        return result
    elif (var_type==1):
        #double
        result = np.ndarray((var_size),dtype=np.float64)
        libpygio.read_gio_double(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_double)),field_count,rank)
        return result
    elif (var_type==2):
        #int32
        result = np.ndarray((var_size),dtype=np.int32)
        libpygio.read_gio_int32(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_int32)),field_count,rank)
        return result
    elif (var_type==3):
        #int64
        result = np.ndarray((var_size),dtype=np.int64)
        libpygio.read_gio_int64(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_int64)),field_count,rank)
        return result        
    elif (var_type==4):
        #uint16
        result = np.ndarray((var_size,field_count),dtype=np.uint16)
        libpygio.read_gio_uint16(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_uint16)),field_count,rank)
        return result


def get_scalars(file_name):
    # Ger the number of scalar and their names
    num_scalars = get_num_scalars(file_name)

    scalars = []
    for s in range(num_scalars):
        scalar_name = get_scalar_name(file_name, s)
        scalars.append(scalar_name)

    return num_scalars, scalars



def create_dataframe(file_name, var_names, rank=-1):
    # create a dataframe from some scalars and a file
    df = pd.DataFrame()

    index = 0
    for scalar in var_names:
        data = read(file_name, scalar, rank)
        df.insert(index, scalar, data[0])
        index = index + 1

    return df




# scalar
def has_scalar(file_name, var_name):
    # Check if a scalar var_name exists in the file file_name
    if sys.version_info[0] == 3:
        file_name=bytes(file_name,'ascii')
        var_name=bytes(var_name,'ascii')

    var_size = libpygio.get_elem_num(file_name)
    var_type = libpygio.get_scalar_type(file_name,var_name)
    return var_type!=10


def get_num_scalars(file_name):
    # Get the number of scalars 
    return ( libpygio.get_num_scalars( file_name.encode('ascii') ) )


def get_num_elements(file_name, rank_id=-1):
    return ( libpygio.get_elem_num( file_name.encode('ascii'), rank_id ) )


def get_scalar_name(file_name, i):
    # Get the name of the scalar at index i
    libpygio.get_scalar_name.restype = ct.POINTER(ct.c_char)
    temp_str = libpygio.get_scalar_name(file_name.encode('ascii'), i)

    return ( (ct.string_at(temp_str)).decode('ascii') )



def get_num_ranks(file_name):
    # Get the number of ranks in a file
    return ( libpygio.get_num_ranks( file_name.encode('ascii') ) )



def get_num_ranks_in(file_name, extents):
    # Get the number of ranks in a 3D extents[minX, maxX, minY, maxY, minZ, maxZ]
    exts = (ct.c_int * len(extents))(*extents)
    return ( libpygio.get_num_ranks_in( file_name.encode('ascii'), exts ) )


def get_ranks_in(file_name, extents):
    # Get the ranks in a 3D extents[minX, maxX, minY, maxY, minZ, maxZ]
    exts = (ct.c_int * len(extents))(*extents)
    num_ranks = libpygio.get_num_ranks_in( file_name.encode('ascii'), exts )


    result = np.ndarray((num_ranks),dtype=np.int32)
    result.ctypes.data_as(ct.POINTER(ct.c_int32))
    
    result = libpygio.get_ranks_in( file_name.encode('ascii'), exts )

    return result, num_ranks




def inspect_gio(file_name):
    print("~gio_inspect file_name", file_name)
    if sys.version_info[0] == 3:
        file_name=bytes(file_name,'ascii')

    print("gio_inspect file_name", file_name)
    libpygio.inspect_gio(file_name)





#
# Octree
def get_octree(file_name):
    libpygio.get_scalar.restype = ct.POINTER(ct.c_char)
    temp_str = libpygio.read_octree_scalar(file_name)

    return ct.string_at(temp_str)


def read_octree_scalar(file_name, var_names, leaf_id=-1):
    # Generic read function, works for octree or full file
    ret = []
    if not isinstance(var_names,list):
        var_names = [ var_names ]

    print("leaf_id:", leaf_id)

    if leaf_id == -1:
        for var_name in var_names:
            ret.append( gio_read_(file_name, var_name, -1) )
    else:
        for var_name in var_names:
            ret.append( gio_read_oct(file_name, var_name, leaf_id) )

    return np.array( ret )


def read_oct(file_name, var_name, leaf_id):
    var_size = libpygio.get_elem_num_in_leaf(file_name, leaf_id)
    var_type = libpygio.get_scalar_type(file_name,var_name)

    if(var_type==10):
        print ("scalar not found")
        return
    elif(var_type==9):
        print ("scalar type not known (not int32/int64/float/double)")
    elif(var_type==0):
        #float
        result = np.ndarray((var_size),dtype=np.float32)
        libpygio.read_gio_oct_float(file_name, leaf_id, var_name, result.ctypes.data_as(ct.POINTER(ct.c_float)))
        return result
    elif(var_type==1):
        #double
        result = np.ndarray((var_size),dtype=np.float64)
        libpygio.read_gio_oct_double(file_name, leaf_id, var_name, result.ctypes.data_as(ct.POINTER(ct.c_double)))
        return result
    elif(var_type==2):
        #int32
        result = np.ndarray((var_size),dtype=np.int32)
        libpygio.read_gio_oct_int32(file_name, leaf_id, var_name, result.ctypes.data_as(ct.POINTER(ct.c_int32)))
        return result
    elif(var_type==3):
        #int64
        result = np.ndarray((var_size),dtype=np.int64)
        libpygio.read_gio_oct_int64(file_name, leaf_id, var_name, result.ctypes.data_as(ct.POINTER(ct.c_int64)))
        return result 



#def get_octree_leaves(file_name, extents):
#    exts = (ct.c_int * len(extents))(*extents)
#
#    num_leaves = libpygio.get_num_octree_leaves(file_name, exts)
#
#    result = np.ndarray((num_leaves),dtype=np.int32)
#    result.ctypes.data_as(ct.POINTER(ct.c_int32))
#    
#    result = libpygio.get_octree_leaves( file_name.encode('ascii'), exts )
#
#    return num_leaves, result


#read = gio_read
#has_scalar = gio_has_scalar
#inspect = gio_inspect
