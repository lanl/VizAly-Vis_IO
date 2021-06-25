import numpy as np
from mpi4py import MPI
from numba import jit
import sys
import csv

sys.path.append("/projects/exasky/pascal-projects/genericio/python")
import genericio as gio

def read_single_col_csv(filename):
	with open(filename, 'r') as file:
		the_list = file.read().split('\n')
	
	the_list.pop(0)	# Remove header
	the_list.pop()	# Remove last item which is empty. Why is that empty? who knows????
	return the_list


def read_csv(filename, seperator=','):
	with open(filename, 'r') as read_obj:
		# pass the file object to reader() to get the reader object
		csv_reader = csv.reader(read_obj, delimiter=seperator)
		# Pass reader object to list() to get a list of lists
		list_of_rows = list(csv_reader)
	
		return list_of_rows


def write_csv(filename, list_of_list, header):
	with open(filename, "w", newline="") as f:
		writer = csv.writer(f)
		writer.writerow(header)
		writer.writerows(list_of_list)


def getUserInput(argv):
	idListFile = ""
	particleFile = ""
	outputFileName = "id_list.csv"


	try:
		idListFile = argv[1]
		particleFile = argv[2]
		outputFileName = argv[3]
	except:
		print("Could not read in the id liet file and/or particle file and/or output file name")
		exit(1)

	return idListFile, particleFile, outputFileName



def initMPI():
	'''Get rank and world size'''
	comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()

	return comm, rank, size



@jit(nopython=True)
def findParticle(search_id_list, id_list):
	found_list = []

	index = 0
	for id in id_list:
		for search_id in search_id_list:
			if search_id == id:
				found_list.append(index)
				break
		
		index = index +  1

	return found_list


@jit(nopython=True)
def extractVariable(index_list, var, num_items):
	var_list = []

	for index in range(num_items):
		var_list.append(var[index])

	return var_list



def main(argv):
	comm, rank, worldSize = initMPI()
	idListFile, particleFile, outputFileName = getUserInput(argv)

	if rank == 0:
		print(idListFile)
		print(particleFile)
		print(outputFileName)


	# Read Id list
	search_id_list = read_single_col_csv(idListFile)
	_search_id_array = np.array(search_id_list)
	search_id_array = _search_id_array. astype(int)

	if rank == 0:
		print("# particles:", len(search_id_array))

	
	total_gio_ranks = gio.get_num_ranks(particleFile)
	#print("num gio ranks:", total_gio_ranks)

	num_gio_ranks_per_mpi_rank = int(total_gio_ranks/worldSize)
	#print("num_gio_ranks_per_mpi_rank:", num_gio_ranks_per_mpi_rank)


	# Determine num gio ranks to read per MPI ranks
	my_gio_ranks_start = num_gio_ranks_per_mpi_rank * rank
	my_gio_ranks_end = my_gio_ranks_start + num_gio_ranks_per_mpi_rank

	if rank == worldSize:
		my_gio_ranks_end = total_gio_ranks


	variables = ["x", "y", "z", "vx", "vy", "vz", "phi", "id", "mask"]
	
	
	# Read Each gio rank
	for gio_rank in range(my_gio_ranks_start, my_gio_ranks_end):
		found_particles = []

		id_list = gio.gio_read(particleFile, "id", gio_rank)
		#print(rank, "~", gio_rank, ":", len(id_list[0]))
		#print(type(search_id_array[0]))
		#print(type(id_list[0]))


		found_id_list = findParticle(search_id_array, id_list[0])
		found_id_array = np.array(found_id_list)


		if len(found_id_list) > 0:
			print(rank, "~ Gio rank:", gio_rank, ", #items", len(found_id_list))

		
		if len(found_id_list) != 0:
			print(rank, " ~ gio rank:", gio_rank, ", #particles: ", len(found_id_list))
			#print(found_id_list)

			for v in variables:
				print("Variable:", v)
				var  = gio.gio_read(particleFile, v, gio_rank)
				var_found_list = extractVariable(found_id_array, var[0], len(found_id_list))
				#print(var_found_list)

				found_particles.append(var_found_list)

		if found_particles != []:
			print(rank, "~ #particles", len(found_particles[0]))
			#print(found_particles)

			transposed_list = list(map(list, zip(*found_particles)))

			write_csv("_temp_particles_halo_" + outputFileName + "_" + str(gio_rank) + "_" + str(rank)+".csv", transposed_list, variables)
		
	


if __name__ == "__main__":
	main(sys.argv)




# mpirun -n 8 python extractParticlesUsingIDs.py _temp_particle_ids_halo457489976_orig_1_1.csv /projects/groups/vizproject/dssdata/cosmo/Argonne_L360_HACC001/STEP499/m000.full.mpicosmo.499 orig_457489976
# mpirun -n 8 python extractParticlesUsingIDs.py _temp_particle_ids_halo457489976_vel_.01_1_1.csv /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-499/reduction/cbench/decompressed_files/sz_PosNoComp_vel_.01__m000.full.mpicosmo.499 _vel_.01_457489976