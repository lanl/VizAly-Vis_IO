import numpy as np
import pygio
import statistics
import csv
import numba
from numba import jit



#@jit(nopython=True)
def write_csv(filename, list_name, list_data):
	with open(filename, 'w') as f:
		# using csv.writer method from CSV package
		write = csv.writer(f)
		
		write.writerow(list_name)
		write.writerows(list_data)


#@jit(nopython=True, parallel=True)
def match_halos_by_id(orig_list, test_list, num_items, match_list):
	
	for i in range(num_items):
		found =  False
		if i%10000 == 0:
			print(i)
		#print(i)
		for j in range(num_items):
			if orig_list[i] == test_list[j]:
				match_list[i] = j
				found = True
				break
		
		if found == False:
			print("At: ", i, "halo tag: ", orig_list[i], " not found!")

	print("List processing done!")

	write_csv("matching_haloID.csv", "matching_id", match_list)



#@jit(nopython=True)
def main():
	data_orig = pygio.read_genericio("/home/pascalgrosset/data/cosmo/vel_analysis/orig-499.haloproperties", ["fof_halo_tag"])
	data_blosc = pygio.read_genericio("/home/pascalgrosset/data/cosmo/vel_analysis/BLOSC_-499.haloproperties", ["fof_halo_tag"])

	data_orig__halo_tag = data_orig["fof_halo_tag"]
	data_blosc__halo_tag = data_blosc["fof_halo_tag"]

	print(data_orig__halo_tag[0])
	print(data_blosc__halo_tag[0])
	match_list = np.empty([len(data_orig__halo_tag)], dtype=int)
	match_halos_by_id(data_orig__halo_tag, data_blosc__halo_tag, int(len(data_orig__halo_tag)), match_list)
	

if __name__ == "__main__":
	main()