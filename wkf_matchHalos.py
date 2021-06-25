from wkf_utilities import *
from wkf_gioUtils import *

"""
Number of Elements: 845185
Total number of Elements: 18446744073709551615
[data type] Variable name
---------------------------------------------
[i 32] fof_halo_count
[i 64] fof_halo_tag
[f 32] fof_halo_mass
[f 32] fof_halo_ke
[f 32] fof_halo_center_x
[f 32] fof_halo_center_y
[f 32] fof_halo_center_z
[f 32] fof_halo_angmom_x
[f 32] fof_halo_angmom_y
[f 32] fof_halo_angmom_z
[f 32] fof_halo_max_cir_vel
[f 32] fof_halo_com_x
[f 32] fof_halo_com_y
[f 32] fof_halo_com_z
[f 32] fof_halo_mean_vx
[f 32] fof_halo_mean_vy
[f 32] fof_halo_mean_vz
[f 32] fof_halo_vel_disp
[f 32] fof_halo_1D_vel_disp
[i 64] sod_halo_count
[f 32] sod_halo_radius
[f 32] sod_halo_mass
[f 32] sod_halo_ke
[f 32] sod_halo_1d_vdisp
[f 32] sod_halo_max_cir_vel
[f 32] sod_halo_min_pot_x
[f 32] sod_halo_min_pot_y
[f 32] sod_halo_min_pot_z
[f 32] sod_halo_angmom_x
[f 32] sod_halo_angmom_y
[f 32] sod_halo_angmom_z
[f 32] sod_halo_com_x
[f 32] sod_halo_com_y
[f 32] sod_halo_com_z
[f 32] sod_halo_mean_vx
[f 32] sod_halo_mean_vy
[f 32] sod_halo_mean_vz
[f 32] sod_halo_vel_disp
[f 32] sod_halo_cdelta
[f 32] sod_halo_cdelta_error
[f 32] sod_halo_c_acc_mass
[f 32] sod_halo_c_peak_mass

(i=integer,f=floating point, number bits size)
"""



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

	if rank == 0:
		print("num_orig_items:", num_orig_items)
		print("num_comp_items:", num_comp_items)
		print("toleranceSq:", toleranceSq)
		print("orig_coord[0]:", orig_coord[0])
		print("comp_coord[0]:", comp_coord[0])
		print("len(orig_coord[0]:", len(orig_coord[0]))
		print("len(comp_coord[0]:", len(comp_coord[0]))

	

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

	print(rank, "~ List processing done!")

	return match_list





def main(argv):
	comm, rank, worldSize = initMPI()
	
	json_data = readJsonFile(argv)

	
	# Read compressed file

	# vars_comp = readData(json_data["inputs"]["comparison-file"], json_data["inputs"]["comparison-variables"])
	# coord_comp_tmp = np.array([vars_comp[0], vars_comp[1], vars_comp[2]])
	# coord_comp = np.transpose(coord_comp_tmp)
	# num_items_read = len(vars_comp[0])

	# vars_comp = read_distributed(json_data["inputs"]["comparison-file"], )
	# print("num_items_read:", num_items_read, " at rank ", rank)

	vars_comp, num_items_read = read_distributed(json_data["inputs"]["comparison-file"], json_data["inputs"]["comparison-variables"], rank, worldSize)

	coord_comp =  list_to_m_array(vars_comp)


	# Read all of the original file
	#vars = readData(json_data["inputs"]["orig-file"], json_data["inputs"]["comparison-variables"])
	vars = read_all(json_data["inputs"]["orig-file"], json_data["inputs"]["comparison-variables"])
	pos = list_to_m_array(vars)
	totalRead = get_num_elements(json_data["inputs"]["orig-file"])




	# Get variable count from each rank
	numReadPerRank = np.array( num_items_read )
	recvEachRank = np.zeros(worldSize, dtype=int)

	comm.Allgather([numReadPerRank, MPI.INT], [recvEachRank, MPI.INT])
	totalRead = sum(recvEachRank)

	
	# compute offsets
	offsets = []
	offsets.append(0)
	for i in range(worldSize-1):
		offsets.append( recvEachRank[i] + offsets[i] )

	print("rank:", rank, ", gather:",recvEachRank, ", total: ",totalRead, ", offsets:", offsets)




	# # Get orig Values from each rank
	# pos_recv = np.zeros((3, totalRead), dtype=np.float32) 
	# for i in range(3):
	# 	comm.Allgatherv([vars[i], MPI.FLOAT], [pos_recv[i], recvEachRank, offsets, MPI.FLOAT])
	# pos = np.transpose(pos_recv)


	# match halos
	print("Match halos")
	match_list = np.empty(num_items_read, dtype=int)
	match_list = match_halos_by_position(rank, pos, totalRead, coord_comp,num_items_read, json_data["inputs"]["tolerance"], match_list)

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


# mpirun -n 4 python wkf_matchHalos.py inputs/comparisonInput.json