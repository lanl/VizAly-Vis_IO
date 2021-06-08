import numpy as np
import pygio
import statistics
import csv
import numba
from numba import jit

from utils import *



#@jit(nopython=True)
def main():
    data_orig = readData("/home/pascalgrosset/data/cosmo/vel_analysis/orig-499.haloproperties", ["fof_halo_mass"])
    var = getArray(data_orig, 0)


	data_orig__halo_tag = data_orig["fof_halo_tag"]
	data_blosc__halo_tag = data_blosc["fof_halo_tag"]

	print(data_orig__halo_tag[0])
	print(data_blosc__halo_tag[0])
	match_list = np.empty([len(data_orig__halo_tag)], dtype=int)
	match_halos_by_id(data_orig__halo_tag, data_blosc__halo_tag, int(len(data_orig__halo_tag)), match_list)
	

if __name__ == "__main__":
	main()