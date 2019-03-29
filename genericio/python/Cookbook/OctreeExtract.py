import sys
import numpy as np
import pandas as pd
sys.path.append("/home/pascal/projects/VizAly-Vis_IO/genericio/python")
import genericio as gio


## Input Section
input_file_name = "/bigData/Halos/b0168/octree-m001-499.bighaloparticles"
octree_region = [197,198, 199,200, 175,176]
query = "['fof_halo_tag'] == 767287898"
display_values = ['x', 'y', 'z']
output_filename = "halo_767287898.csv"



##### Script Starts #####

# Find leaves with data
extents = np.array(octree_region)
num_leaves, leaves = gio.gio_get_octree_leaves(input_file_name, extents)
print "\nnum_leaves", num_leaves

for i in range(num_leaves):
	print leaves[i]


# Read leaf into pandas
num_vars = gio.gio_get_num_variables(input_file_name)
df2 = pd.DataFrame()
for i in range(num_vars):
	var_name = gio.gio_get_variable(input_file_name, i)

	for l in range(num_leaves):
		data = gio.gio_read_oct(input_file_name, var_name, leaves[l])
		df2.insert(i, var_name, data)


# Extract Halo
query_string = "df2"+query
row_selection = eval(query_string)
col_selection = display_values

df2.loc[row_selection, col_selection].to_csv(output_filename, index=False)

