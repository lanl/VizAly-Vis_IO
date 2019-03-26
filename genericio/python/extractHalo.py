import sys
import numpy as np
import pandas as pd
import genericio as gio
import dask.dataframe as dd
import time


def get_data_frame(file_name)
	# Get number of variables
	num_vars = gio.gio_get_num_variables(file_name)

	# Read in data for each variable
	df = pd.DataFrame()
	for i in range(num_vars):
		var_name = gio.gio_get_variable(file_name, i)
		data = gio.gio_read(file_name, var_name)
		df.insert(i, var_name, data.tolist())

	return df


def main(argv):
	file_name = sys.argv[1]

	# Load the data into a dataframe
	df = pd.DataFrame()
	df = get_data_frame(file_name)


if __name__ == "__main__":
	main(sys.argv[1:])