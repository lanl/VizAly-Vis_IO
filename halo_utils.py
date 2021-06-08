from utils import *

def adrianVecDiffMag(v1, v2):
    '''Comparison is with Adrian comparison metric'''
    return ( np.linalg.norm(v1-v2)/ np.linalg.norm(v1) )


def computeMaxDiffMag(indexFile, data_orig_gio, data_comp_gio, numItems):
    '''Create a list of tuples for ang mom diff'''
    data_orig__ang_mom_x = np.array(data_orig_gio["fof_halo_angmom_x"])
    data_orig__ang_mom_y = np.array(data_orig_gio["fof_halo_angmom_y"])
    data_orig__ang_mom_z = np.array(data_orig_gio["fof_halo_angmom_z"])
    
    data_comp__ang_mom_x = np.array(data_comp_gio["fof_halo_angmom_x"])
    data_comp__ang_mom_y = np.array(data_comp_gio["fof_halo_angmom_y"])
    data_comp__ang_mom_z = np.array(data_comp_gio["fof_halo_angmom_z"])
    
    diffMagList = []

    for i in range(numItems):
        index_comp = i
        index_orig = int(indexFile[index_comp])
        
        vec_orig = np.array([data_orig__ang_mom_x[index_orig], data_orig__ang_mom_y[index_orig], data_orig__ang_mom_z[index_orig]])
        vec_comp = np.array([data_comp__ang_mom_x[index_comp], data_comp__ang_mom_y[index_comp], data_comp__ang_mom_z[index_comp]])
    
        magDiff = adrianVecDiffMag(vec_orig, vec_comp)
#         if i == 0:
#             print(vec_orig, vec_comp, magDiff)
#             print(index_orig, index_comp)
        
        diffMagList.append( (i,magDiff) )
        
    return diffMagList



def sortTuple(tup, rev=True): 
    '''Sort a list of tuples in descending order'''
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using second element of 
    # sublist lambda has been used 
    return(sorted(tup, key = lambda x: x[1], reverse=rev))



def getFilteredList(data, sortedList, index_file, num_elem, filter_var_name, filter_var_value, num_to_show, orig=True):
    '''Filter the list based on > filter val'''
    fullList_orig=[]
    count = 0
    
    for i in range(num_elem):
        _index_to_show = sortedList[i][0]
        if orig == True:
            __index_to_show = int(index_file[_index_to_show])
        else:
            __index_to_show = _index_to_show
        
        
        if data[filter_var_name][__index_to_show] > filter_var_value:
            #print(data_show_orig_gio[filter_var_name][__index_to_show])
            
            theList = []
            theList.append(sortedList[i][1])
            for var in variables_to_show:
                theList.append(data[var][__index_to_show])

            fullList_orig.append(theList)

            count = count + 1
            if num_to_show != -1:
                if count > num_to_show:
                    break
    
    return fullList_orig



def getParticles(data, variables, num_particles, var_filter_name, var_filter_value):
    '''Find particles that match a halo tag'''

    rows = []
    for p in range(num_particles):
        if data[var_filter_name][p] == var_filter_value:
            var_values = [] 
            for var in variables:
                var_values.append( data[var] )
            
            rows.append(var_values)
        
    return rows
