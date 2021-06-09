from utils import *

"""
particletag
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



@jit(nopython=True)
def find_particle_id(halo_tag, data_array, num_elements):
	list_of_ids = []
	for i in range(num_elements):
		if data_array[i][1] == halo_tag:
			list_of_ids.append(data_array[i][0])
	
	return list_of_ids



def main(argv):
	# Initialize
	comm, rank, worldSize = initMPI()
	haloID, haloparticleTagFile, outputFileName = getUserInput(argv)

	if rank == 0:
		print(haloID)
		print(haloparticleTagFile)
		print(outputFileName)

	# Read data per rank
	data = readData(haloparticleTagFile)
	varNames = getVariableNames(haloparticleTagFile)

	# Put the data into arrays for faster access
	_halo_particle = np.array([data[0], data[1]])
	halo_particle = np.transpose(_halo_particle)

	# Find the ids of the particles in the halo
	list_of_ids = find_particle_id(int(haloID), halo_particle, len(halo_particle))

	num_local_elements = len(list_of_ids)
	#print("rank:", rank, "  num_local_elements:", num_local_elements)


	# Gather the number of elements to grab from each rank
	gather_data = num_local_elements
	gather_data = comm.gather(gather_data, root=0)

	# Gather all ids on rank 0
	num_global_elements = 0
	if rank == 0:
		#print("gather_data:", gather_data)
		num_global_elements = sum(gather_data)
		#print("num_global_elements:",num_global_elements)



	offsets = []
	full_id_list = None
	if rank == 0:
		offsets.append(0)
		for i in range(worldSize-1):
			offsets.append( gather_data[i] + offsets[i] )
		full_id_list = np.empty(num_global_elements, dtype=int)

		#print("offsets:",offsets)
		#print("gather_data:",gather_data)

	send_buff = np.array(list_of_ids)
	comm.Gatherv(send_buff, [full_id_list, gather_data, offsets, MPI.LONG], root=0)


	# Write to file
	if rank == 0:
		np.savetxt(outputFileName, full_id_list, delimiter=',', fmt='%d', header="id",comments='')


if __name__ == "__main__":
	main(sys.argv)

# mpirun -n 4 python findParticleIDs.py 812471593 /home/pascalgrosset/data/cosmo/vel_analysis/orig-499.haloparticletags 812471593_orig.csv


"""
# worst small halo ID: 812471593 (53 particles)
# worst medium halo ID: 543655151 (1616 particles)
#worst large halo ID: 944404240 (13913 particles)
"""