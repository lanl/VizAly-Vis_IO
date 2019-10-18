import sys
import argparse
import matplotlib
import numpy as np
import pandas as pd
import dask.dataframe as dd

sys.path.append("/Users/pascalgrosset/projects/VizAly-Vis_IO/genericio/python")
import genericio as gio


## Input Section
if len(sys.argv) > 1:
	input_file_name = sys.argv[1]
else:
	input_file_name = "/Users/pascalgrosset/data/Argonne/Halos/STEP499/m000-499.fofproperties"


query = "['fof_halo_count'] > 300000"
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



result = eval("dd[(dd" + query + ")]")   #result = dd[(dd['fof_halo_count'] > 300000)]
dfout = result.compute()

print (dfout[display_values])


# python Query.py /Users/pascalgrosset/data/Argonne/Halos/STEP499/m000-499.fofproperties
