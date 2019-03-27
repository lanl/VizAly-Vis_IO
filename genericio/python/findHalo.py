import sys
import numpy as np
import genericio as gio
import pandas as pd


file_name = "/datastore/Cosmology/b0168/octree-m001-499.sodproperties"

num_vars = gio.gio_get_num_variables(file_name)

# Read in data for each variable
df = pd.DataFrame()
for i in range(num_vars):
	var_name = gio.gio_get_variable(file_name, i)
	data = gio.gio_read(file_name, var_name)
	df.insert(i, var_name, data)

row_selection = df['fof_halo_count'] > 400000
#col_selection = ['fof_halo_tag', 'fof_halo_count', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_y', 'fof_halo_mean_x', 'fof_halo_mean_y', 'fof_halo_mean_z']
col_selection = ['fof_halo_tag']
print( df.loc[row_selection, col_selection] )



file_name = "/datastore/Cosmology/b0168/octree-m001-499.haloparticles"

extents = np.array([197,198, 199,200, 175,176])
num_leaves, temp = gio.gio_get_octree_leaves(file_name, extents)
print "num_leaves", num_leaves

for i in range(num_leaves):
	print temp[i]


