import sys
import numpy as np
import pandas as pd
import genericio as gio
import dask.dataframe as dd
import time

from mpi4py import MPI


comm = MPI.COMM_WORLD
rank = comm.Get_rank()
#from dask.distributed import Client


file_name = sys.argv[1]

num_vars = gio.gio_get_num_variables(file_name)
print (num_vars)

# Read in data for each variable
df = pd.DataFrame()
for i in range(num_vars):
	var_name = gio.gio_get_variable(file_name, i)
	data = gio.gio_read(file_name, var_name)
	df.insert(i, var_name, data.tolist())


ddf = dd.from_pandas(df, npartitions=8)

start = time.time()
print  ( ddf['fof_halo_count'].max().compute() )
end = time.time()
print "Max time taken: " + str(end - start) + "s"



#print  ( df['fof_halo_count'].min() )

#print ( df.sort_values(['fof_halo_count'], ascending=False) )

# create a dask data frame




#client = Client()
#ddf2 = client.persist( dd.from_pandas(df, npartitions=16 ) )


#start = time.time()
#print  ( ddf2['fof_halo_count'].max().compute() )
#end = time.time()
#print "Max time taken: " + str(end - start) + "s"
