import numpy as np
from numpy.lib.function_base import median
import statistics
import csv
import numba
import sys
import json
from numba import jit
from mpi4py import MPI
import pandas as pd
import os
import fnmatch


from wkf_utilities import *


def find_files(path, pattern):
	file_list = []
	for path, subdirs, files in os.walk(path):
		for file in files:
			if fnmatch.fnmatch(file, pattern):
				file_list.append(os.path.join(path, file))

	return file_list


def read_single_col_csv(filename):
	with open(filename, 'r') as file:
		the_list = file.read().split('\n')
	
	the_list.pop(0)	# Remove header
	the_list.pop()	# Remove last item which is empty. Why is that empty? who knows????
	return the_list



def combine_csv(filenames, merged_file):
	combined_csv = pd.concat( [ pd.read_csv(f) for f in filenames ] )
	combined_csv.to_csv(merged_file, index=False )
	return combined_csv


def write_single_col_csv_header(filename, my_list, header):
	with open(filename, "w", newline="") as f:
		writer = csv.writer(f)
		writer.writerow(header)
		for val in my_list:
			writer.writerow([val])

def write_csv_header(filename, my_list, header):
	with open(filename, "w", newline="") as f:
		writer = csv.writer(f)
		writer.writerow(header)
		for val in my_list:
			writer.writerow(val)



def write_csv(filename, list_name, list_data):
	with open(filename, 'w') as f:
		# using csv.writer method from CSV package
		write = csv.writer(f)
		
		write.writerow(list_name)
		write.writerows(list_data)



def write_array_csv(filename, list_name, list_data):
	np.savetxt(filename, list_data, delimiter=',', header=list_name)



def read_csv(filename, seperator=','):
	with open(filename, 'r') as read_obj:
		# pass the file object to reader() to get the reader object
		csv_reader = csv.reader(read_obj, delimiter=seperator)
		# Pass reader object to list() to get a list of lists
		list_of_rows = list(csv_reader)
	
		return list_of_rows



def read_csv_to_array(filename, seperator):
	temp_list = read_csv(filename, seperator)
	arr = np.array(temp_list[0])
	return arr

	

def readJsonFile(filename):
	'''Read JSON file'''
	with open(filename) as f:
		json_data = json.load(f)
	return json_data



def getArray(variableList, pos):
	'''Coz Arrays are faster than lists'''
	arr = np.array( variableList[pos] )
	return arr



def getPosArray(variableList, pos):
	'''Coz Arrays are faster than lists'''
	coord_comp_tmp = np.array([ variableList[ pos[0] ], variableList[ pos[1] ], variableList[ pos[2] ] ] )
	coord_comp = np.transpose(coord_comp_tmp)
	return coord_comp

   

def sortTuple(tup, rev=True): 
	'''Sort a list of tuples in descending order'''
	# reverse = None (Sorts in Ascending order) 
	# key is set to sort using second element of 
	# sublist lambda has been used 
	return(sorted(tup, key = lambda x: x[1], reverse=rev))



def initMPI():
	'''Get rank and world size'''
	comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()

	return comm, rank, size



def getRankSize():
	'''Get rank and world size'''
	comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()

	return rank, size

