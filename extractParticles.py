from utils import *

"""
Number of Elements: 1073726359
Total number of Elements: 18446744073709551615
[data type] Variable name
---------------------------------------------
[f 32] x
[f 32] y
[f 32] z
[f 32] vx
[f 32] vy
[f 32] vz
[f 32] phi
[i 64] id
[i 16] mask
"""

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



@jit(nopython=True)
def find_particles(idList, particles, num_elements):
	list_of_particles = []

	for i in range(num_elements):
		localList = []
		for id in idList:
			print(id)
			if particles[i][7] == int(id):
				for v in range(8):
					localList.append( particles[i][v] )
				break

	return list_of_particles
	



def main(argv):
	# Initialize
	comm, rank, worldSize = initMPI()
	idListFile, particleFile, outputFileName = getUserInput(argv)

	# Read Id list
	idList = read_csv(idListFile)

	#print(idList)

	# Read data per rank
	data = readData(particleFile)
	varNames = getVariableNames(particleFile)

	print("data file read")

	# # Put the data into arrays for faster access
	# _particles = np.array( data[0],data[1],data[2],  data[3],data[4],data[5], data[6], data[7], data[8] )
	# particles = np.transpose(_particles)

	# # Find the ids of the particles in the halo
	# list_of_particles = find_particles(idList, particles, len(particles))


	# print(list_of_particles)


	# # Gather the number of elements to grab from each rank
	# num_local_elements = len(list_of_particles[0])
	# local_elements_list = np.zeros(worldSize)
	# comm.gather(local_elements_list, root=0)


	# # Gather all ids on rank 0
	# num_global_elements = sum(local_elements_list)

	# num_arrays = len(local_elements_list)

	# offsets = []
	# offsets.append(0)
	# for i in range(worldSize-1):
	# 	offsets.append( local_elements_list[i] + offsets[i] )

	# all_data = []
	# for i in range(num_arrays):
	# 	all_data.append(np.empty(num_global_elements, dtype=int))
	# 	comm.Gatherv(list_of_particles[i], [all_data[i], local_elements_list, offsets, MPI.LONG], 0)


	# # Write to file
	# if rank == 0:
	# 	write_csv(outputFileName,["x","y","z", "vx", "vy", "vz", "phi", "id", "mask"], list_of_ids)


if __name__ == "__main__":
	main(sys.argv)

# mpirun -n 4 python extractParticles.py results/812471593_orig.csv /home/pascalgrosset/data/cosmo/vel_analysis/m000.full.mpicosmo.499 812471593_particles_orig.csv
