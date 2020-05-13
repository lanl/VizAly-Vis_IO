#!/usr/bin/python

import sys
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

def randrange(n, vmin, vmax):
    '''
    Helper function to make an array of random numbers having shape (n, )
    with each number distributed Uniform(vmin, vmax).
    '''
    return (vmax - vmin)*np.random.rand(n) + vmin

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')



filename = sys.argv[1]
numPoints = int(sys.argv[2])

myArray_x = np.fromfile(filename, dtype=np.float32, count=numPoints, offset=0)
myArray_y = np.fromfile(filename, dtype=np.float32, count=numPoints, offset=numPoints*4)
myArray_z = np.fromfile(filename, dtype=np.float32, count=numPoints, offset=(numPoints+numPoints)*4)


fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')


ax.scatter(myArray_x, myArray_y, myArray_z, c='r', marker='o')

ax.set_xlabel('X Label')
ax.set_ylabel('Y Label')
ax.set_zlabel('Z Label')

plt.show()


# Run as 
# python python/verifyOutput.py <input_file_name> <num_particles_in_file>
# python python/verifyOutput.py /Users/pascalgrosset/Desktop/genericio/testOutput_255.raw 5991271