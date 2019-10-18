import sys 
import argparse 
import matplotlib 
import numpy as np 
import pandas as pd 
import dask.dataframe as dd
sys.path.append("/Users/pascalgrosset/projects/VizAly-Vis_IO/genericio/python") 
import genericio as gio

particle_file_name = "/Users/pascalgrosset/data/Argonne/STEP499/m000.full.mpicosmo.499"


extents = [ 197,198, 198,199, 163,164]
num_ranks, ranks = gio.get_ranks_in(particle_file_name, extents)
print(ranks)
selected_rank_particle_df = gio.create_dataframe(particle_file_name, ['id','x','y','z'], ranks)
print(selected_rank_particle_df)


halo_partilce_tags = "/Users/pascalgrosset/data/Argonne/Halos/STEP499/m000-499.haloparticletags"
num_hpt_scalars, hpt_scalars = gio.get_scalars(halo_partilce_tags)
print(hpt_scalars)

halo_part_tags_df = gio.create_dataframe(halo_partilce_tags, hpt_scalars)
max_tag_df = halo_part_tags_df[(halo_part_tags_df['fof_halo_tag'] == 762035800)]


joined = dd.merge(selected_rank_particle_df, max_tag_df, on='id')
joined.to_csv("halo_762035800.csv", sep=',', index=False)