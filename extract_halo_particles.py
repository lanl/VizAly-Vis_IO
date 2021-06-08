from utils import *


def getRankSize():
	'''Get rank and world size'''
	comm = MPI.COMM_WORLD
	rank = comm.Get_rank()
	size = comm.Get_size()

	return rank, size


def readJsonFile(filename):
	'''Read JSON file'''
	with open(filename) as f:
		json_data = json.load(f)
	return json_data


def readData(filename, variables):
	'''Read variable'''
	data = pygio.read_genericio(filename, variables)

	vars = []
	for var in variables:
		vars.append( data[var] )
	
	return vars 



@jit(nopython=True)
def compare_pos(rank, coord_orig, coord_comp, toleranceSq):
	'''Compare distance'''

	dist_sq = ( (coord_orig[0]-coord_comp[0]) * (coord_orig[0]-coord_comp[0]) ) + \
			  ( (coord_orig[1]-coord_comp[1]) * (coord_orig[1]-coord_comp[1]) ) + \
			  ( (coord_orig[2]-coord_comp[2]) * (coord_orig[2]-coord_comp[2]) )

	# if rank == 0:
	# 	print("coord_orig:",coord_orig)
	# 	print("coord_comp:",coord_comp)
	# 	print("dist_sq:",dist_sq)


	if dist_sq < toleranceSq:
		return True
	
	return False


@jit(nopython=True)
def match_halos_by_position(rank, orig_coord, num_orig_items, comp_coord, num_comp_items, tolerance, match_list):
	'''Compare position for halos'''

	toleranceSq = tolerance*tolerance

	# if rank == 0:
	# 	print("num_orig_items:", num_orig_items)
	# 	print("num_comp_items:", num_comp_items)
	# 	print("toleranceSq:", toleranceSq)

	

	for i in range(num_comp_items):
		found =  False

		# if i%1000 == 0:
		# 	print(i, " of ", num_comp_items)

		for j in range(num_orig_items):

			# if rank == 0:
			# 	print("\norig_coord[",j,"]:", orig_coord[j])
			# 	print("comp_coord[",i,"]:", comp_coord[i])

			if compare_pos(rank, orig_coord[j], comp_coord[i], toleranceSq):
				match_list[i] = j
				found = True
				break
		
		if found == False:
			print("At: ", i, " not found!")

	print("List processing done!")

	return match_list





def main(argv):
	comm = MPI.COMM_WORLD
	rank, worldSize = getRankSize()

	
	json_data = readJsonFile(argv)
    filename = json_data["inputs"]["data"]["filename"]
    particles_id_list_file = json_data["inputs"]["id-list"]["filename"]


    # Read File
    data = pygio.read_genericio(filename)
	
	


	# Read original file
	vars = readData(json_data["inputs"]["orig-file"], json_data["inputs"]["comparison-variables"])


	# Get variable count from each rank
	numReadPerRank = np.array( num_items_read )
	recvEachRank = np.zeros(worldSize, dtype=np.int)

	comm.Allgather([numReadPerRank, MPI.INT], [recvEachRank, MPI.INT])
	totalRead = sum(recvEachRank)

	
	# compute offsets
	offsets = []
	offsets.append(0)
	for i in range(worldSize-1):
		offsets.append( recvEachRank[i] + offsets[i] )


	print("rank:", rank, "gather:",recvEachRank, " total: ",totalRead, "offsets:", offsets)


	# initialize an array with 0
	pos_recv = np.zeros((3, totalRead), dtype=np.float32) 


	# Get orig Values from each rank
	for i in range(3):
		comm.Allgatherv([vars[i], MPI.FLOAT], [pos_recv[i], recvEachRank, offsets, MPI.FLOAT])
	pos = np.transpose(pos_recv)


	# match halos
	print("Match halos")
	match_list = np.empty(num_items_read, dtype=int)
	match_list = match_halos_by_position(rank, pos,totalRead, coord_comp,num_items_read, json_data["inputs"]["tolerance"], match_list)

	match_list.tofile("rank_"+str(rank)+".csv", sep=json_data["outputs"]["index-file-seperator"])

	

	# Gather the full list 
	full_match_list = np.empty(totalRead, dtype=int)
	comm.Gatherv(match_list, [full_match_list, recvEachRank, offsets, MPI.LONG], 0)



	# Write list to file
	if rank == 0:
		full_match_list.tofile(json_data["outputs"]["index-file"],sep=json_data["outputs"]["index-file-seperator"])




	MPI.Finalize()



if __name__ == "__main__":
	main(sys.argv[1])


# mpirun -n 4 python matchHalosPos.py comparisonInput.json