import numpy as np
from numpy.lib.function_base import median
import statistics
import csv
import numba
import sys
import json
from numba import jit
from mpi4py import MPI

import pygio


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

	

def getVariableNames(filename):
	'''Get variable names'''
	# instantiate a GenericIO class
	gio_file = pygio.PyGenericIO(filename)
	gio_vars = gio_file.get_variables()

	variables = []
	for var in gio_vars:
		variables.append(var.name)

	return variables


def readData(filename, variables=[]):
	'''Read variable'''
	if variables == []:
		variables = getVariableNames(filename)

	data = pygio.read_genericio(filename, variables)

	vars = []
	for var in variables:
		vars.append( data[var] )
	
	return vars 







def readJsonFile(filename):
	'''Read JSON file'''
	with open(filename) as f:
		json_data = json.load(f)
	return json_data



def getRankSize():
	'''Get rank and world size'''
	comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()

	return rank, size


def initMPI():
	'''Get rank and world size'''
	comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()

	return comm, rank, size


def getArray(variableList, pos):
	'''Coz Arrays are faster than lists'''
	arr = np.array( variableList[pos] )
	return arr



def getPosArray(variableList, pos):
	'''Coz Arrays are faster than lists'''
	coord_comp_tmp = np.array([ variableList[ pos[0] ], variableList[ pos[1] ], variableList[ pos[2] ] ] )
	coord_comp = np.transpose(coord_comp_tmp)
	return coord_comp

   



class VarStats:
	def __init(self, array):
		self.array = array
		self.computeStats(array)


	def computeStats(self, array):
		self._mean = np.mean(array)
		self._median = np.median(array)
		self._min = np.min(array)
		self. _max = np.max(array)
		self._stdev = np.stdev(array)
		self._count = len(array)

	
	def createHistogram(self, numBins):
		self._histogram = np.histogram(self.array, numBins)

