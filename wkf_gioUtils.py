"""
Using the "old" python GenericIO interface to read in particles. 
"""
import numpy as np
import sys

sys.path.append("/projects/exasky/pascal-projects/genericio/python")
import genericio as gio


def get_num_ranks(filename):
	"""Get the number of GenericIO ranks""" 
	return ( gio.get_num_ranks(filename) )


def get_num_elements(filename):
	return gio.get_num_elements(filename)


def get_scalars(filename):
	num_scalars, scalars = gio.get_scalars(filename)

def read_variable_rank(filename, variable, rank):
	"""Returns a numpy array of the variable for rank"""
	return ( gio.gio_read(filename, variable, rank)[0] )


def read_variables(filename, variables, rank):
	"""Returns all the variables from list variables from a rank"""
	var_list = []
	for var in variables:
		var_list.append( read_variable_rank(filename, var, rank) )
	
	return var_list


def get_my_gio_reading_range(numGioRanks, numMPIRanks, myRank):
	"""Returns the number of Gio ranks with the first rank and up to the last rank"""
	num_gio_ranks_per_mpi_rank = int(numGioRanks / numMPIRanks)

	my_gio_ranks_start = num_gio_ranks_per_mpi_rank * myRank
	my_gio_ranks_end = my_gio_ranks_start + num_gio_ranks_per_mpi_rank

	if myRank == numMPIRanks-1:
		my_gio_ranks_end = numGioRanks

	return my_gio_ranks_start, my_gio_ranks_end




def read_all(filename, variables):
	vars_read = read_variables(filename, variables, -1)
	return vars_read



def read_distributed(filename, variables, myRank, numMPIRanks):
	num_gio_Ranks = get_num_ranks(filename)
	my_gio_ranks_start, my_gio_ranks_end = get_my_gio_reading_range(num_gio_Ranks, numMPIRanks, myRank)
	print(myRank, " ~ ", my_gio_ranks_start , " - ", my_gio_ranks_end)
	
	num_vars = len(variables)
	

	vars_read = read_variables(filename, variables, my_gio_ranks_start)
	print(len(vars_read))
	print(len(vars_read[0]))
	num_elements = len(vars_read[0])

	for g in range(my_gio_ranks_start+1, my_gio_ranks_end):
		_vars_read = read_variables(filename, variables, g)
		num_elements = num_elements + len(_vars_read[0])

		# add
		for i in range(num_vars):
			vars_read[i] = np.concatenate((vars_read[i], _vars_read[i]))

	return vars_read, num_elements



def list_to_m_array(the_list):
	return np.column_stack(the_list)