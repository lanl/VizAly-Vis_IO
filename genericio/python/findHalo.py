import sys
import numpy as np
import genericio as gio
import pandas as pd
import csv

from plotly.offline import iplot
import plotly.graph_objs as go



file_name = "/bigData/Halos/b0168/octree-m001-499.fofproperties"

num_vars = gio.gio_get_num_variables(file_name)

# Read in data for each variable
df = pd.DataFrame()
for i in range(num_vars):
	var_name = gio.gio_get_variable(file_name, i)
	data = gio.gio_read(file_name, var_name)
	df.insert(i, var_name, data)

row_selection = df['fof_halo_count'] > 400000
col_selection = ['fof_halo_tag', 'fof_halo_count', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_y']
print( df.loc[row_selection, col_selection] )

# fof_halo_tag  fof_halo_count  fof_halo_center_x  fof_halo_center_y  fof_halo_center_y
#  767287898          714950         197.450500         198.124374         198.124374
#  41580266          427328          10.065739         169.862503         169.862503
#  103664932          432487          22.558313         223.289932         223.289932
#  246319855          604872          61.002811         228.239975         228.239975
#  254127552          546000          67.653236          99.019066          99.019066
#  390141673          501737          97.826874           8.504021           8.504021
#  441282178          494088         108.118942         227.779297         227.779297
#  774622410          518906         187.236099         206.409576         206.409576
#  767287898          714950         197.450500         198.124374         198.124374





#  767287898          714950         197.450500         198.124374         198.124374
file_name = "/bigData/Halos/b0168/octree-m001-499.bighaloparticles"

extents = np.array([197,198, 199,200, 175,176])
num_leaves, temp = gio.gio_get_octree_leaves(file_name, extents)
print "\nnum_leaves", num_leaves

for i in range(num_leaves):
	print temp[i]

leaf = temp[i]
num_vars = gio.gio_get_num_variables(file_name)

# read leaf into pandas
df2 = pd.DataFrame()
for i in range(num_vars):
	var_name = gio.gio_get_variable(file_name, i)

	data = gio.gio_read_oct(file_name, var_name, leaf)
	df2.insert(i, var_name, data)



#  767287898          714950         197.450500         198.124374         198.124374
row_selection = df2['fof_halo_tag'] == 767287898
col_selection = ['x', 'y', 'z']
#print( df2.loc[row_selection, col_selection] )

df2.loc[row_selection, col_selection].to_csv("halo_767287898.csv", index=False)



#$ time python python/findHalo.py
#        fof_halo_tag  fof_halo_count  fof_halo_center_x  fof_halo_center_y  fof_halo_center_y
#74153       41580266          427328          10.065739         169.862503         169.862503
#91143      103664932          432487          22.558313         223.289932         223.289932
#217642     246319855          604872          61.002811         228.239975         228.239975
#269207     254127552          546000          67.653236          99.019066          99.019066
#335691     390141673          501737          97.826874           8.504021           8.504021
#432906     441282178          494088         108.118942         227.779297         227.779297
#620389     774622410          518906         187.236099         206.409576         206.409576
#724787     767287898          714950         197.450500         198.124374         198.124374

#num_leaves 1
#1745
