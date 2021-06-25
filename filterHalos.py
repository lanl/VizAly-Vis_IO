from gioUtils import *
from utilities import *

variables_to_show = ["fof_halo_count"
,"fof_halo_tag"
,"fof_halo_mass"
,"fof_halo_ke"
,"fof_halo_center_x"
,"fof_halo_center_y"
,"fof_halo_center_z"
,"fof_halo_angmom_x"
,"fof_halo_angmom_y"
,"fof_halo_angmom_z"
,"fof_halo_max_cir_vel"
,"fof_halo_com_x"
,"fof_halo_com_y"
,"fof_halo_com_z"
,"fof_halo_com_vx"
,"fof_halo_com_vy"
,"fof_halo_com_vz"
,"fof_halo_1D_vel_disp"]


# Read in the data
data_orig_gio = pygio.read_genericio(orig_file, variables_to_show)
data_comp_gio = pygio.read_genericio(comp_file, variables_to_show)

# Read in the mathched list of halos created from matchHalosPos.py
index_file = read_csv_to_array('results/new_Hacc_indices_com_sz_PosNoComp_vel_.01-499.csv',',')


# Find the halos with the largest difference in ang mom diff
theList = computeMaxDiffMag(index_file, data_orig_gio, data_comp_gio, len(index_file))

# Sort the list in desc order of difference in ang mom diff
sortedList = sortTuple(theList)


num_to_show = 100
num_elem = len(index_file)

filter_var_name = "fof_halo_count"
filter_var_value = 0


# Use above list to get a list of halos sorted by ang mom diff and filtered by size of halos
_fullList_orig = getFilteredList(data_orig_gio, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, True)
_fullList_comp = getFilteredList(data_comp_gio, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, False)


# Create data frames for easier manipulation
# fof_halo_vel_disappeared
#,"fof_halo_mean_vx"
#,"fof_halo_mean_vy"
#,"fof_halo_mean_vz"
#cols=['mag_diff','fof_halo_count', 'fof_halo_tag', 'fof_halo_mass', 'fof_halo_ke', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z', 'fof_halo_angmom_x', 'fof_halo_angmom_y', 'fof_halo_angmom_z', 'fof_halo_max_cir_vel', 'fof_halo_com_x', 'fof_halo_com_y', 'fof_halo_com_z', 'fof_halo_mean_vx', 'fof_halo_mean_vy', 'fof_halo_mean_vz', 'fof_halo_vel_disp', 'fof_halo_1D_vel_disp']
cols=['mag_diff','fof_halo_count', 'fof_halo_tag', 'fof_halo_mass', 'fof_halo_ke', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z', 'fof_halo_angmom_x', 'fof_halo_angmom_y', 'fof_halo_angmom_z', 'fof_halo_max_cir_vel', 'fof_halo_com_x', 'fof_halo_com_y', 'fof_halo_com_z', 'fof_halo_com_vx', 'fof_halo_com_vy', 'fof_halo_com_vz', 'fof_halo_1D_vel_disp']


_df_orig = pd.DataFrame(_fullList_orig, columns=cols)
_df_comp = pd.DataFrame(_fullList_comp, columns=cols)

_df_orig.to_csv("results/new_hacc_largest_100_ang_mom_diff__orig-499.csv")
_df_comp.to_csv("results/new_hacc_largest_100_ang_mom_diff__PosNoComp_vel_.01-499.csv") 




num_to_show = -1
num_elem = len(index_file)

filter_var_name = "fof_halo_count"
filter_var_value = 10000


# Use above list to get a list of halos sorted by ang mom diff and filtered by size of halos
_fullList_orig = getFilteredList(data_orig_gio, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, True)
_fullList_comp = getFilteredList(data_comp_gio, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, False)


# Create data frames for easier manipulation
# Create data frames for easier manipulation
# fof_halo_vel_disappeared
#,"fof_halo_mean_vx"
#,"fof_halo_mean_vy"
#,"fof_halo_mean_vz"
#cols=['mag_diff','fof_halo_count', 'fof_halo_tag', 'fof_halo_mass', 'fof_halo_ke', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z', 'fof_halo_angmom_x', 'fof_halo_angmom_y', 'fof_halo_angmom_z', 'fof_halo_max_cir_vel', 'fof_halo_com_x', 'fof_halo_com_y', 'fof_halo_com_z', 'fof_halo_mean_vx', 'fof_halo_mean_vy', 'fof_halo_mean_vz', 'fof_halo_vel_disp', 'fof_halo_1D_vel_disp']
cols=['mag_diff','fof_halo_count', 'fof_halo_tag', 'fof_halo_mass', 'fof_halo_ke', 'fof_halo_center_x', 'fof_halo_center_y', 'fof_halo_center_z', 'fof_halo_angmom_x', 'fof_halo_angmom_y', 'fof_halo_angmom_z', 'fof_halo_max_cir_vel', 'fof_halo_com_x', 'fof_halo_com_y', 'fof_halo_com_z', 'fof_halo_com_vx', 'fof_halo_com_vy', 'fof_halo_com_vz', 'fof_halo_1D_vel_disp']

_df_orig = pd.DataFrame(_fullList_orig, columns=cols)
_df_comp = pd.DataFrame(_fullList_comp, columns=cols)

_df_orig.to_csv("results/new_hacc_largest_above_10000_ang_mom_diff__orig-499.csv")
_df_comp.to_csv("results/new_hacc_largest_above_10000_ang_mom_diff__PosNoComp_vel_.01-499.csv")