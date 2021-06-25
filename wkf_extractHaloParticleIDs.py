from wkf_utilities import *
from wkf_gioUtils import *

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
	outputName = ""


	try:
		haloID = argv[1]
		haloparticleTagFile = argv[2]
		outputName = argv[3]
	except:
		print("Could not read in haloID and/or halo particletag file and/or output file name")
		exit(1)

	return haloID, haloparticleTagFile, outputName



@jit(nopython=True)
def find_particle_id(halo_tag, halo_id_data_array, id_data_array, num_elements):
	list_of_ids = []
	for i in range(num_elements):
		if halo_id_data_array[i] == halo_tag:
			list_of_ids.append(id_data_array[i])
	
	return list_of_ids




def main(argv):
	# Initialize
	comm, rank, worldSize = initMPI()
	haloID, haloparticleTagFile, outputName = getUserInput(argv)

	if rank == 0:
		print(haloID)
		print(haloparticleTagFile)
		print(outputName)


	total_gio_ranks = get_num_ranks(haloparticleTagFile)
	my_gio_ranks_start, my_gio_ranks_end = get_my_gio_reading_range(total_gio_ranks, worldSize, rank)


	variables = ["id", "fof_halo_tag"]
	temp_output_name_prefix = "_temp_particle_ids_halo_" + haloID + "_"  

	# Read each gio rank
	for gio_rank in range(my_gio_ranks_start, my_gio_ranks_end):
		found_particles = []

		fof_halo_tag_list = gio.gio_read(haloparticleTagFile, "fof_halo_tag", gio_rank)
		id_list = gio.gio_read(haloparticleTagFile, "id", gio_rank)
		print(rank, "~", len(fof_halo_tag_list[0]))

		found_id_list = find_particle_id(int(haloID), fof_halo_tag_list[0], id_list[0], len(fof_halo_tag_list[0]))

		if len(found_id_list) > 0:
			print("# ids:", len(found_id_list))
			write_single_col_csv_header(temp_output_name_prefix + str(rank) + "__"+ str(gio_rank) + ".csv", found_id_list, ["id"])


	# Merge all the files together
	comm.Barrier()
	if rank == 0:
		#cur_path = os.path.abspath(os.getcwd())
		print(cur_path)
		files_to_merge = find_files(cur_path, temp_output_name_prefix+"*")
		#print(files_to_merge)
		#if len(files_to_merge) > 1:
		combine_csv(files_to_merge, "results/" + outputName)
		#else:
		#	os.system('cp ' + files_to_merge[0] + ' ' +  "results/" + outputName + ".csv")


if __name__ == "__main__":
	main(sys.argv)

# mpirun -n 8 python wkf_extractHaloParticleIDs.py 4  
# mpirun -n 8 python wkf_extractHaloParticleIDs.py 457489976 /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-499/analysis/halo/sz_PosNoComp_vel_.01-499.haloparticletags 457489976_vel_.01



"""
# worst small halo ID: 812471593 (53 particles)
# worst medium halo ID: 543655151 (1616 particles)
#worst large halo ID: 944404240 (13913 particles)
"""