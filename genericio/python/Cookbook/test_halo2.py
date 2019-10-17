import sys 
import argparse 
import matplotlib 
import numpy as np 
import pandas as pd 
import dask.dataframe as dd
sys.path.append("/home/pascal/projects/VizAly-Vis_IO/genericio/python") 
import genericio as gio  

halo_file_name="/bigData/b0168/m001-499.fofproperties"


num_scalars = gio.get_num_scalars(halo_file_name)
print(num_scalars)


scalar_name = gio.get_scalar_name(halo_file_name, 0)
print(scalar_name)

scalar_name = gio.get_scalar_name(halo_file_name, 1)
print(scalar_name)


num_ranks = gio.get_num_ranks(halo_file_name)
print(num_ranks)

num_elems = gio.get_num_elements("/bigData/STEP499/m000.full.mpicosmo.499")
print("/bigData/STEP499/m000.full.mpicosmo.499 has", num_elems)

num_elems = gio.get_num_elements("/bigData/STEP499/m000.full.mpicosmo.499", 218)
print("/bigData/STEP499/m000.full.mpicosmo.499 int rank 218 has", num_elems)

extents = [30, 34,  30, 34,  62, 66]
num_ranks_in = gio.get_num_ranks_in(halo_file_name, extents)
print(num_ranks_in)


ranks_in, num_ranks_in= gio.get_ranks_in(halo_file_name, extents)
print(num_ranks_in)
for r in  range(num_ranks_in):
	print(ranks_in[r])

gio.inspect_gio(halo_file_name)


particle_tag_file = "/bigData/b0168/m001-499.haloparticletags"
df = pd.DataFrame()

gio.inspect_gio(halo_file_name)

gio.inspect_gio(particle_tag_file)


