import sys
import argparse
import matplotlib
import numpy as np
import pandas as pd
import dask.dataframe as dd

sys.path.append("/home/pascal/projects/VizAly-Vis_IO/genericio/python")
import genericio as gio


## Input Section
if len(sys.argv) > 1:
	input_file_name = sys.argv[1]
else:
	input_file_name = "/bigData/b0168/m001-499.fofproperties"
	#input_file_name = "/bigData/b0168/octree-m001-499.sodproperties"
	#input_file_name = "/bigData/b0168/octree-m001-499.fofproperties"
	#input_file_name = "/bigData/b0168/m001-499.sodproperties"


query = "['fof_halo_count'] > 200000"
display_values = ['fof_halo_tag', 'fof_halo_count', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_y']


##### Script Starts #####
num_vars = gio.gio_get_num_variables(input_file_name)

# Read in data for each variable
df = pd.DataFrame()
for i in range(num_vars):
	var_name = gio.gio_get_variable(input_file_name, i)

	data = gio.gio_read(input_file_name, var_name)
	df.insert(i, var_name, data[0])

dd = dd.from_pandas(df, npartitions=4)


#query_string = "df"+query
#row_selection = eval(query_string)
col_selection = display_values

#print( df.loc[row_selection, col_selection] )


result = dd[(dd['fof_halo_count'] > 300000)]
dfout = result.compute()
print (dfout)
print (dfout.loc[col_selection])
