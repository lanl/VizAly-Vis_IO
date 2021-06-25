import numpy as np
from mpi4py import MPI
from numba import jit
import sys
import csv

sys.path.append("/projects/exasky/pascal-projects/genericio/python")
import genericio as gio

"""
particletag
[data type] Variable name
---------------------------------------------
[i 64] id
[i 64] fof_halo_tag

(i=integer,f=floating point, number bits size)
"""
def write_csv(filename, my_list, header):
	with open(filename, "w", newline="") as f:
		writer = csv.writer(f)
		writer.writerow(header)
		for val in my_list:
			writer.writerow([val])




def getUserInput(argv):
	haloID = -1
	haloparticleTagFile = ""
	outputFileName = "id_list.csv"


	try:
		haloID = argv[1]
		haloparticleTagFile = argv[2]
		outputFileName = argv[3]
	except:
		print("Could not read in haloID and/or halo particletag file and/or output file name")
		exit(1)

	return haloID, haloparticleTagFile, outputFileName



@jit(nopython=True)
def find_particle_id(halo_tag, halo_id_data_array, id_data_array, num_elements):
	list_of_ids = []
	for i in range(num_elements):
		if halo_id_data_array[i] == halo_tag:
			list_of_ids.append(id_data_array[i])
	
	return list_of_ids



def initMPI():
	'''Get rank and world size'''
	comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()

	return comm, rank, size



def main(argv):
	# Initialize
	comm, rank, worldSize = initMPI()
	haloID, haloparticleTagFile, outputFileName = getUserInput(argv)

	if rank == 0:
		print(haloID)
		print(haloparticleTagFile)
		print(outputFileName)


	total_gio_ranks = gio.get_num_ranks(haloparticleTagFile)
	#print("num gio ranks:", total_gio_ranks)

	num_gio_ranks_per_mpi_rank = int(total_gio_ranks/worldSize)
	#print("num_gio_ranks_per_mpi_rank:", num_gio_ranks_per_mpi_rank)


	# Determine num gio ranks to read per MPI ranks
	my_gio_ranks_start = num_gio_ranks_per_mpi_rank * rank
	my_gio_ranks_end = my_gio_ranks_start + num_gio_ranks_per_mpi_rank

	if rank == worldSize:
		my_gio_ranks_end = total_gio_ranks


	variables = ["id", "fof_halo_tag"]


	# Read Each gio rank
	for gio_rank in range(my_gio_ranks_start, my_gio_ranks_end):
		found_particles = []

		fof_halo_tag_list = gio.gio_read(haloparticleTagFile, "fof_halo_tag", gio_rank)
		id_list = gio.gio_read(haloparticleTagFile, "id", gio_rank)
		print(rank, "~", len(fof_halo_tag_list[0]))

		found_id_list = find_particle_id(int(haloID), fof_halo_tag_list[0], id_list[0], len(fof_halo_tag_list[0]))

		if len(found_id_list) > 0:
			print("# ids:", len(found_id_list))
			write_csv("_temp_particle_ids_halo" + outputFileName + "_" + str(gio_rank)+"_"+str(rank)+".csv", found_id_list, ["id"])



if __name__ == "__main__":
	main(sys.argv)

# mpirun -n 8 python extractParticlesUsingHaloIDs.py 457489976 /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-499/analysis/halo/orig-499.haloparticletags 457489976_orig
# mpirun -n 8 python extractParticlesUsingHaloIDs.py 457489976 /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-499/analysis/halo/sz_PosNoComp_vel_.01-499.haloparticletags 457489976_vel_.01



"""
# worst small halo ID: 812471593 (53 particles)
# worst medium halo ID: 543655151 (1616 particles)
#worst large halo ID: 944404240 (13913 particles)
"""