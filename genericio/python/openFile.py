from  __future__ import print_function

import sys
import numpy as np
import pandas as pd
import genericio as gio
import dask.dataframe as dd
import time



def query(df):
	row_selection = df['fof_halo_count'] > 400000
	col_selection = ['fof_halo_tag', 'fof_halo_count', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z', 'sod_halo_count', 'sod_halo_radius']
	print( df.loc[row_selection, col_selection] )



def queryExamples(df):
	print ( df['fof_halo_count'].max() )

	print ( df.nlargest(2, ['fof_halo_count'] ) )


	## Query All
	# option 1
	filter = df['fof_halo_count'] > 400000
	print( df[filter] )

	#option 2
	print ( df.loc[ df['fof_halo_count'] > 400000 ] )


	## Query Selected
	# option 1
	print( df.loc[ df['fof_halo_count'] > 400000, ['fof_halo_tag', 'fof_halo_count', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z']] )

	# option 2
	row_selection = df['fof_halo_count'] > 400000
	col_selection = ['fof_halo_tag', 'fof_halo_count', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z']
	print( df.loc[row_selection, col_selection] )



def diag(df):
	print ( df.dtypes )
	print ( df.info() )
	print ( df.shape)




def get_data_frame(file_name):
	print( "Reading " + file_name + " ... ", end="")

	# Get number of variables
	num_vars = gio.gio_get_num_variables(file_name)

	# Read in data for each variable
	df = pd.DataFrame()
	for i in range(num_vars):
		var_name = gio.gio_get_variable(file_name, i)
		data = gio.gio_read(file_name, var_name)
		df.insert(i, var_name, data)

	print( "done!")

	return df




def main(argv):
	file_name = sys.argv[1]

	# Load the data into a dataframe
	df = pd.DataFrame()
	df = get_data_frame(file_name)

	query(df)
	


if __name__ == "__main__":
	main(sys.argv[1:])
