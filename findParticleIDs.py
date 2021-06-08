from utils import *

"""
[data type] Variable name
---------------------------------------------
[i 64] id
[i 64] fof_halo_tag

(i=integer,f=floating point, number bits size)
"""

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



@jit(nopython=True, parallel=True)
def find_particle_id(halo_tag, data_array, num_elements):
	list_of_ids = []
	for i in range(num_elements):
		if data_array[i][0] == halo_tag:
			list_of_ids.append(data_array[i][1])
	
	return list_of_ids



def main(argv):
	# Initialize
	comm, rank, worldSize = initMPI()
	haloID, haloparticleTagFile, outputFileName = getUserInput(argv)

	# Read data per rank
	readData = readData(haloparticleTagFile)
	varNames = getVariableNames(haloparticleTagFile)

	# Put the data into arrays for faster access
	_halo_particle = np.array([readData[0], readData[1]])
	halo_particle = np.transpose(_halo_particle)

	# Find the ids of the particles in the halo
	list_of_ids = find_particle_id(haloID, halo_particle, len(halo_particle))


	# Gather the number of elements to grab from each rank
	num_local_elements = len(list_of_ids)
	local_elements_list = np.zeros(worldSize)
	comm.gather(local_elements_list, root=0)


	# Gather all ids on rank 0
	num_global_elements = sum(local_elements_list)

	offsets = []
	offsets.append(0)
	for i in range(worldSize-1):
		offsets.append( local_elements_list[i] + offsets[i] )

	full_id_list = np.empty(num_global_elements, dtype=int)
	comm.Gatherv(list_of_ids, [full_id_list, local_elements_list, offsets, MPI.LONG], 0)


	# Write to file
	if rank == 0:
		write_csv(outputFileName,"id",list_of_ids)


if __name__ == "__main__":
	main(sys.argv)
