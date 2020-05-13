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

  void read_gio_float(char* file_name, char* var_name, float* data, int field_count){
    read_gio<float>(file_name,var_name,data,field_count);
  }
  void read_gio_double(char* file_name, char* var_name, double* data, int field_count){
    read_gio<double>(file_name,var_name,data,field_count);
  }
  void read_gio_uint16(char* file_name, char* var_name, uint16_t* data, int field_count){
    read_gio<uint16_t>(file_name,var_name,data,field_count);
  }
  void read_gio_int32(char* file_name, char* var_name, int* data, int field_count){
    read_gio<int>(file_name,var_name,data,field_count);
  }
  void read_gio_int64(char* file_name, char* var_name, int64_t* data, int field_count){
    read_gio<int64_t>(file_name,var_name,data,field_count);
  }
  
  int64_t get_elem_num(char* file_name){
    gio::GenericIO reader(file_name);
    reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
    int num_ranks = reader.readNRanks();
    uint64_t size = 0;
    for(int i =0;i<num_ranks;++i)
      size +=reader.readNumElems(i);
    reader.close();
    return size;
  }

  var_type get_variable_type(char* file_name,char* var_name){
   gio::GenericIO reader(file_name);
   std::vector<gio::GenericIO::VariableInfo> VI;
   reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
   reader.getVariableInfo(VI);

   int num =VI.size();
    for(int i =0;i<num;++i){
      gio::GenericIO::VariableInfo vinfo = VI[i];
      if(vinfo.Name == var_name){
	if(vinfo.IsFloat && vinfo.ElementSize == 4)
	  return float_type;
	else if(vinfo.IsFloat && vinfo.ElementSize == 8)
	  return double_type;
        else if(!vinfo.IsFloat && vinfo.ElementSize == 2)
          return uint16_type;
	else if(!vinfo.IsFloat && vinfo.ElementSize == 4)
	  return int32_type;
	else if(!vinfo.IsFloat && vinfo.ElementSize == 8)
	  return int64_type;
	else
	  return type_not_found;
      }
    }
    return var_not_found;
      
  }

  int get_variable_field_count(char* file_name,char* var_name){
   gio::GenericIO reader(file_name);
   std::vector<gio::GenericIO::VariableInfo> VI;
   reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
   reader.getVariableInfo(VI);

   int num =VI.size();
    for(int i =0;i<num;++i){
      gio::GenericIO::VariableInfo vinfo = VI[i];
      if(vinfo.Name == var_name) {
        return vinfo.Size/vinfo.ElementSize;
      }
    }
    return 0;
  }

extern "C" void inspect_gio(char* file_name){
  int64_t size = get_elem_num(file_name);
  gio::GenericIO reader(file_name);
  std::vector<gio::GenericIO::VariableInfo> VI;
  reader.openAndReadHeader(gio::GenericIO::MismatchAllowed);
  reader.getVariableInfo(VI);
  std::cout<<"Number of Elements: "<<size<<std::endl;
  int num =VI.size();
  std::cout<<"[data type] Variable name"<<std::endl;
  std::cout<<"---------------------------------------------"<<std::endl;
  for(int i =0;i<num;++i){
    gio::GenericIO::VariableInfo vinfo = VI[i];

    if(vinfo.IsFloat)
      std::cout<<"[f";
    else
      std::cout<<"[i";
    int NumElements = vinfo.Size/vinfo.ElementSize;
    std::cout<<" "<<vinfo.ElementSize*8;
    if (NumElements > 1)
      std::cout<<"x"<<NumElements;
    std::cout<<"] ";
    std::cout<<vinfo.Name<<std::endl;
  }
  std::cout<<"\n(i=integer,f=floating point, number bits size)"<<std::endl;
}

