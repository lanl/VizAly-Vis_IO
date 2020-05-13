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

from __future__ import print_function
import numpy as np
import ctypes as ct
import os,sys

#Define where the library is and load it
_path = os.path.dirname(__file__)
libpygio = ct.CDLL(_path + '/../frontend/libpygio.so')
#we need to define the return type ("restype") and
#the argument types
libpygio.get_elem_num.restype=ct.c_int64
libpygio.get_elem_num.argtypes=[ct.c_char_p]

libpygio.get_variable_type.restype=ct.c_int
libpygio.get_variable_type.argtypes=[ct.c_char_p,ct.c_char_p]

libpygio.get_variable_field_count.restype=ct.c_int
libpygio.get_variable_field_count.argtypes=[ct.c_char_p,ct.c_char_p]

libpygio.read_gio_uint16.restype=None
libpygio.read_gio_uint16.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_uint16),ct.c_int]

libpygio.read_gio_int32.restype=None
libpygio.read_gio_int32.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_int),ct.c_int]

libpygio.read_gio_int64.restype=None
libpygio.read_gio_int64.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_int64),ct.c_int]

libpygio.read_gio_float.restype=None
libpygio.read_gio_float.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_float),ct.c_int]

libpygio.read_gio_double.restype=None
libpygio.read_gio_double.argtypes=[ct.c_char_p,ct.c_char_p,ct.POINTER(ct.c_double),ct.c_int]

libpygio.inspect_gio.restype=None
libpygio.inspect_gio.argtypes=[ct.c_char_p]

def gio_read_(file_name,var_name):
    if sys.version_info[0] == 3:
        file_name = bytes(file_name,'ascii')
        var_name = bytes(var_name,'ascii')
    var_size = libpygio.get_elem_num(file_name)
    var_type = libpygio.get_variable_type(file_name,var_name)
    field_count = libpygio.get_variable_field_count(file_name,var_name)
    if(var_type==10):
        print("Variable not found")
        return
    elif(var_type==9):
        print("variable type not known (not uint16/int32/int64/float/double)")
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
    elif(var_type==4):
        #uint16
        result = np.ndarray((var_size,field_count),dtype=np.uint16)
        libpygio.read_gio_uint16(file_name,var_name,result.ctypes.data_as(ct.POINTER(ct.c_uint16)),field_count)
        return result

def gio_read(file_name,var_names):
    ret = []
    if not isinstance(var_names,list):
        var_names = [ var_names ]
    for var_name in var_names:
        ret.append( gio_read_(file_name,var_name) )
    return np.array( ret )

def gio_has_variable(file_name,var_name):
    if sys.version_info[0] == 3:
        file_name=bytes(file_name,'ascii')
        var_name=bytes(var_name,'ascii')
    var_size = libpygio.get_elem_num(file_name)
    var_type = libpygio.get_variable_type(file_name,var_name)
    return var_type!=10

def gio_inspect(file_name):
    if sys.version_info[0] == 3:
        file_name=bytes(file_name,'ascii')
    libpygio.inspect_gio(file_name)

read = gio_read
has_variable = gio_has_variable
inspect = gio_inspect

