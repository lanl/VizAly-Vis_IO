from wkf_utilities import *
from wkf_gioUtils import *

def getUserInput(argv):
	idListFile = ""
	particleFile = ""
	haloId = ""
	outputFileName = "id_list.csv"


	try:
		idListFile = argv[1]
		particleFile = argv[2]
		haloId = argv[3]
		outputFileName = argv[4]
	except:
		print("Could not read in the id list file and/or particle file and/or output file name")
		exit(1)

	return idListFile, particleFile, haloId, outputFileName




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
	idListFile, particleFile, haloId, outputFileName = getUserInput(argv)

	if rank == 0:
		print(idListFile)
		print(particleFile)
		print(haloId)
		print(outputFileName)


	# Read Id list
	search_id_list = read_single_col_csv("results/"+idListFile+".csv")
	_search_id_array = np.array(search_id_list)
	search_id_array = _search_id_array. astype(int)

	if rank == 0:
		print("# particles:", len(search_id_array))


	total_gio_ranks = get_num_ranks(particleFile)
	my_gio_ranks_start, my_gio_ranks_end = get_my_gio_reading_range(total_gio_ranks, worldSize, rank)


	variables = ["x", "y", "z", "vx", "vy", "vz", "phi", "id"]
	temp_output_name_prefix = "_temp_particles_halo_" + haloId + "_" 
	
	# Read Each gio rank
	for gio_rank in range(my_gio_ranks_start, my_gio_ranks_end):
		found_particles = []

		#id_list = gio.gio_read(particleFile, "id", gio_rank)[0]
		id_list = read_variable_rank(particleFile, "id", gio_rank)
		#print(rank, "~", gio_rank, ":", len(id_list[0]))
		#print(type(search_id_array[0]))
		#print(type(id_list[0]))


		found_id_list = findParticle(search_id_array, id_list)
		found_id_array = np.array(found_id_list)


		if len(found_id_list) > 0:
			print(rank, "~ Gio rank:", gio_rank, ", #items", len(found_id_list))

		
		if len(found_id_list) != 0:
			#print(rank, " ~ gio rank:", gio_rank, ", #particles: ", len(found_id_list))
			#print(found_id_list)

			for v in variables:
				#print("Variable:", v)
				#var  = gio.gio_read(particleFile, v, gio_rank)
				var  = read_variable_rank(particleFile, v, gio_rank)
				var_found_list = extractVariable(found_id_array, var, len(found_id_list))
				#print(var_found_list)

				found_particles.append(var_found_list)
			#print("found_particles[0]:",found_particles[0])
			#print("found_particles[4]:",found_particles[4])

		if found_particles != []:
			#print(rank, "~ #particles", len(found_particles[0]))
			#print(found_particles)

			#print("found_particles:",found_particles)
			transposed_list = list(map(list, zip(*found_particles)))
			#print("transposed_list:",transposed_list)

			write_csv_header(temp_output_name_prefix + str(rank) + "__" + str(gio_rank), transposed_list, variables)
		
	
	# Merge all the files together
	comm.Barrier()
	if rank == 0:
		cur_path = os.path.abspath(os.getcwd())
		file_to_merge = find_files(cur_path, temp_output_name_prefix + "*")
		combine_csv(file_to_merge, "results/" + outputFileName)


if __name__ == "__main__":
	main(sys.argv)




# mpirun -n 8 python extractParticlesUsingIDs.py _temp_particle_ids_halo457489976_orig_1_1.csv 457489976 /projects/groups/vizproject/dssdata/cosmo/Argonne_L360_HACC001/STEP499/m000.full.mpicosmo.499 orig_457489976
# mpirun -n 8 python extractParticlesUsingIDs.py _temp_particle_ids_halo457489976_vel_.01_1_1.csv 457489976 /projects/groups/vizproject/dssdata/Exasky/vel_run/vel-test-ts-499/reduction/cbench/decompresreadJsonFile