import sys 
import argparse 
import matplotlib 
import numpy as np 
import pandas as pd 
import dask.dataframe as dd
sys.path.append("/Users/pascalgrosset/projects/VizAly-Vis_IO/genericio/python") 
#sys.path.append("/home/pascal/projects/VizAly-Vis_IO/genericio/python")
import genericio as gio

halo_file_name = "/Users/pascalgrosset/data/Argonne/Halos/STEP499/m000-499.fofproperties"
#halo_file_name = "/datastore/Cosmology/Halos/b0168/STEP499/m000-499.fofproperties"
num_halo_scalars, halo_scalars = gio.get_scalars(halo_file_name)
print(halo_scalars)

halo_selected_scalars = ['fof_halo_count', 'fof_halo_tag', 'fof_halo_mass', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z', 'fof_halo_mean_x', 'fof_halo_mean_y', 'fof_halo_mean_z']
halo_df = gio.create_dataframe(halo_file_name, halo_selected_scalars)

filtered_halo_df = halo_df[(halo_df['fof_halo_count'] > 500000)]
print(filtered_halo_df)