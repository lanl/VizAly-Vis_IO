import pandas as pd
import numpy as np
import sys

from wkf_utilities import *
from wkf_gioUtils import *


def adrianVecDiffMag(v1, v2):
	'''Comparison is with Adrian comparison metric:  ||a2 - a1|| / ||a1||: a1=uncompressed, a2=compressed'''
	return ( np.linalg.norm(v2-v1)/ np.linalg.norm(v1) )


def computeMaxDiffMag(indexFile, data_orig_gio, data_comp_gio, numItems):
	'''Create a list of tuples for ang mom diff'''
	data_orig__ang_mom_x = data_orig_gio[7]
	data_orig__ang_mom_y = data_orig_gio[8]
	data_orig__ang_mom_z = data_orig_gio[9]
	
	data_comp__ang_mom_x = data_comp_gio[7]
	data_comp__ang_mom_y = data_comp_gio[8]
	data_comp__ang_mom_z = data_comp_gio[9]
	
	diffMagList = []

	for i in range(numItems):
		index_comp = i
		index_orig = int(indexFile[index_comp])
		
		vec_orig = np.array([data_orig__ang_mom_x[index_orig], data_orig__ang_mom_y[index_orig], data_orig__ang_mom_z[index_orig]])
		vec_comp = np.array([data_comp__ang_mom_x[index_comp], data_comp__ang_mom_y[index_comp], data_comp__ang_mom_z[index_comp]])
	
		magDiff = adrianVecDiffMag(vec_orig, vec_comp)
#         if i == 0:
#             print(vec_orig, vec_comp, magDiff)
#             print(index_orig, index_comp)
		
		diffMagList.append( (i,magDiff) )
		
	return diffMagList




def getFilteredList(data, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, orig=True):
	'''Filter the list based on > filter val'''
	fullList_orig=[]
	count = 0

	num_vars = len(data)
	print(num_vars)
	
	for i in range(num_elem):
		_index_to_show = sortedList[i][0]
		if orig == True:
			__index_to_show = int(index_file[_index_to_show])
		else:
			__index_to_show = _index_to_show
		
		
		if data[filter_var_name][__index_to_show] > filter_var_value:
			#print(data_show_orig_gio[filter_var_name][__index_to_show])
			
			theList = []
			theList.append(sortedList[i][1])
			#for var in variables_to_show:
			for var in range(num_vars):
				theList.append(data[var][__index_to_show])

			fullList_orig.append(theList)

			count = count + 1
			if num_to_show != -1:
				if count > num_to_show:
					break
	
	return fullList_orig




def main(argv):
	json_data = readJsonFile(argv)

	orig_file = json_data["inputs"]["orig-file"]
	comp_file = json_data["inputs"]["comparison-file"]

	variables_to_show = json_data["outputs"]["halo-comparison"]
	


	# Read in the data
	# data_orig_gio = pygio.read_genericio(orig_file, variables_to_show)
	# data_comp_gio = pygio.read_genericio(comp_file, variables_to_show)

	data_orig_gio = read_all(orig_file, variables_to_show)
	data_comp_gio = read_all(comp_file, variables_to_show)

	
	index_file_name = json_data["outputs"]["index-file"]
	seperator = json_data["outputs"]["index-file-seperator"]

	# Read in the mathched list of halos created from matchHalosPos.py
	index_file = read_csv_to_array(index_file_name, seperator)


	# Find the halos with the largest difference in ang mom diff
	theList = computeMaxDiffMag(index_file, data_orig_gio, data_comp_gio, len(index_file))

	# Sort the list in desc order of difference in ang mom diff
	sortedList = sortTuple(theList)


	cols = ["mag_diff"] +  variables_to_show


	# Outputs
	for filter in json_data["halo-filters-output"]:
		num_to_show = filter["num-to-show"]
		num_elem = len(index_file)

		filter_var_name = filter["filter_var_pos"]
		filter_var_value = filter["filter_var_value"]


		# Use above list to get a list of halos sorted by ang mom diff and filtered by size of halos
		_fullList_orig = getFilteredList(data_orig_gio, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, True)
		_fullList_comp = getFilteredList(data_comp_gio, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, False)

		print("len(_fullList_orig):", len(_fullList_orig))

		


		_df_orig = pd.DataFrame(_fullList_orig, columns=cols)
		_df_comp = pd.DataFrame(_fullList_comp, columns=cols)

		_df_orig.to_csv(filter["orig-output"])
		_df_comp.to_csv(filter["comp-output"]) 



if __name__ == "__main__":
	main(sys.argv[1])


# python wkf_haloMetrics.py inputs/comparisonInput.json