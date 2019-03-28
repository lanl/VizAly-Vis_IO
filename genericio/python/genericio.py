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
import numpy as np
import ctypes as ct
import os

#Define where the library is and load it
_path = os.path.dirname(__file__)
libpygio = ct.CDLL(_path + '/../frontend/libpygio.so')
#we need to define the return type ("restype") and
#the argument types
libpygio.get_elem_num.restype=ct.c_int64
libpygio.get_elem_num.argtypes=[ct.c_char_p]

libpygio.get_elem_num_in_leaf.restype=ct.c_int64
libpygio.get_elem_num_in_leaf.argtypes=[ct.c_char_p, ct.c_int]

libpygio.get_variable_type.restype=ct.c_int
libpygio.get_variable_type.argtypes=[ct.c_char_p, ct.c_char_p]

libpygio.get_variable_field_count.restype=ct.c_int
libpygio.get_variable_field_count.argtypes=[ct.c_char_p, ct.c_char_p]


libpygio.read_gio_int32.restype=None
libpygio.read_gio_int32.argtypes=[ct.c_char_p, ct.c_char_p, ct.POINTER(ct.c_int), ct.c_int]

libpygio.read_gio_int64.restype=None
libpygio.read_gio_int64.argtypes=[ct.c_char_p, ct.c_char_p, ct.POINTER(ct.c_int64), ct.c_int]

libpygio.read_gio_float.restype=None
libpygio.read_gio_float.argtypes=[ct.c_char_p, ct.c_char_p, ct.POINTER(ct.c_float), ct.c_int]

libpygio.read_gio_double.restype=None
libpygio.read_gio_double.argtypes=[ct.c_char_p, ct.c_char_p, ct.POINTER(ct.c_double), ct.c_int]



libpygio.read_gio_oct_int32.restype=None
libpygio.read_gio_oct_int32.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_int)]

libpygio.read_gio_oct_int64.restype=None
libpygio.read_gio_oct_int64.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_int64)]

libpygio.read_gio_oct_float.restype=None
libpygio.read_gio_oct_float.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_float)]

libpygio.read_gio_oct_double.restype=None
libpygio.read_gio_oct_double.argtypes=[ct.c_char_p, ct.c_int, ct.c_char_p, ct.POINTER(ct.c_double)]




libpygio.inspect_gio.restype=None
libpygio.inspect_gio.argtypes=[ct.c_char_p]

libpygio.get_octree.restype=ct.c_char_p
libpygio.get_octree.argtypes=[ct.c_char_p]

libpygio.get_variable.restype=ct.c_char_p
libpygio.get_variable.argtypes=[ct.c_char_p, ct.c_int]





libpygio.get_num_octree_leaves.restype=ct.c_int
libpygio.get_num_octree_leaves.argtypes=[ct.c_char_p, ct.POINTER(ct.c_int)]

libpygio.get_octree_leaves.restype=ct.POINTER(ct.c_int)
libpygio.get_octree_leaves.argtypes=[ct.c_char_p, ct.POINTER(ct.c_int)]




def gio_read_oct(file_name, var_name, leaf_id):
    var_size = libpygio.get_elem_num_in_leaf(file_name, leaf_id)
    var_type = libpygio.get_variable_type(file_name,var_name)
    if(var_type==10):
        print ("Variable not found")
        return
    elif(var_type==9):
        print ("variable type not known (not int32/int64/float/double)")
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


def gio_read(file_name, var_name):
    var_size = libpygio.get_elem_num(file_name)
    var_type = libpygio.get_variable_type(file_name,var_name)
    field_count = libpygio.get_variable_field_count(file_name,var_name)
    if(var_type==10):
        print ("Variable not found")
        return
    elif(var_type==9):
        print ("variable type not known (not int32/int64/float/double)")
    elif(var_type==0):
        #float
        result = np.ndarray((var_size),dtype=np.float32)
        libpygio.read_gio_float(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_float)),field_count)
        return result
    elif(var_type==1):
        #double
        result = np.ndarray((var_size),dtype=np.float64)
        libpygio.read_gio_double(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_double)),field_count)
        return result
    elif(var_type==2):
        #int32
        result = np.ndarray((var_size),dtype=np.int32)
        libpygio.read_gio_int32(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_int32)),field_count)
        return result
    elif(var_type==3):
        #int64
        result = np.ndarray((var_size),dtype=np.int64)
        libpygio.read_gio_int64(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_int64)),field_count)
        return result        


def gio_has_variable(file_name,var_name):
    var_size = libpygio.get_elem_num(file_name)
    var_type = libpygio.get_variable_type(file_name,var_name)
    return var_type!=10


def gio_inspect(file_name):
    libpygio.inspect_gio(file_name)


def gio_get_num_variables(file_name):
    return ( libpygio.get_num_variables(file_name) )


def gio_get_variable(file_name, i):
    libpygio.get_variable.restype = ct.POINTER(ct.c_char)
    temp_str = libpygio.get_variable(file_name, i)

    return ct.string_at(temp_str)


def gio_get_octree(file_name):
    libpygio.get_variable.restype = ct.POINTER(ct.c_char)
    temp_str = libpygio.get_octree(file_name)

    return ct.string_at(temp_str)


def gio_get_octree_leaves(file_name, extents):
    exts = (ct.c_int * len(extents))(*extents)

    num_leaves = libpygio.get_num_octree_leaves(file_name, exts)

    result = np.ndarray((num_leaves),dtype=np.int32)
    result.ctypes.data_as(ct.POINTER(ct.c_int32))
    
    result = libpygio.get_octree_leaves( file_name, exts )

    return num_leaves, result
