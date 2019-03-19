#                    Copyright (C) 2015, UChicago Argonne, LLC
#                               All Rights Reserved
# 
#                               Generic IO (ANL-15-066)
#                  Pascal Grosset, Los Alamos National Laboratory
# 
#                              OPEN SOURCE LICENSE
# 
# Under the terms of Contract No. DE-AC02-06CH11357 with UChicago Argonne,
# LLC, the U.S. Government retains certain rights in this software.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#   1. Redistributions of source code must retain the above copyright notice,
#      this list of conditions and the following disclaimer.
# 
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
# 
#   3. Neither the names of UChicago Argonne, LLC or the Department of Energy
#      nor the names of its contributors may be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
# 
# *****************************************************************************
# 
#                                  DISCLAIMER
# THE SOFTWARE IS SUPPLIED "AS IS" WITHOUT WARRANTY OF ANY KIND.  NEITHER THE
# UNTED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR
# UCHICAGO ARGONNE, LLC, NOR ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY,
# EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE
# ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, DATA, APPARATUS,
# PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE
# PRIVATELY OWNED RIGHTS.
# 
# *****************************************************************************


import struct, sys
import string

class GenericIOHeader:
	def __init__ (self):
		self.filetype = ""
		self.headerSize = 0
		self.numElems = 0
		self.dims = []
		self.numVars = 0
		self.varSize = 0
		self.varStart = 0
		self.numRanks = 0
		self.rankSize = 0
		self.rankStart = 0
		self.globalHeaderSize = 0
		self.physOrigin = []
		self.physScale = []
		self.blockSize = 0
		self.blockStart = 0
		self.octreeSize = 0
		self.octreeStart = 0
	

	def printMe(self):
		print("Filetype:", self.filetype )
		print("HeadeSize:", self.headerSize )
		print("#rows:", self.numElems )
		print("Dims:", self.dims[0], self.dims[1], self.dims[2] )
		print("# vars:", self.numVars )
		print("var size:", self.varSize )
		print("var start:", self.varStart )
		print("# ranks:", self.numRanks )
		print("rank size:", self.rankSize )
		print("rank start:", self.rankStart )
		print("global header size:", self.globalHeaderSize )
		print("Origin:", self.physOrigin[0], self.physOrigin[1], self.physOrigin[2] )
		print("Scale:", self.physScale[0], self.physScale[1], self.physScale[2] )
		print("block size:", self.blockSize )
		print("block start:", self.blockStart )
		print("octree size:", self.octreeSize )
		print("octree start:", self.octreeStart )


class GenericIOVariableInfo:
	floatMask  = 1
	signedMask = 2
	phyXMask   = 4
	phyYMask   = 8
	phyZMask   = 16
	ghostMask  = 32

	def __init__(self):
		self.varname = ""
		self.varSize = 0
		self.isFloat = 0
		self.isSigned = 0
		self.isPhysCoordX = 0
		self.isPhysCoordY = 0
		self.isPhysCoordZ = 0
		self.maybePhysGhost = 0

	def printMe(self):
		print("Variable Name:", self.varname)
		print("Variable Size:", self.varSize)
		print("is Float?:", self.isFloat)
		print("is Signed?:", self.isSigned)
		print("is PhysCoord X?:", self.isPhysCoordX)
		print("is PhysCoord Y?:", self.isPhysCoordY)
		print("is PhysCoord Z?:", self.isPhysCoordZ)
		print("maybe Phys Ghost?:", self.maybePhysGhost)
		print("\n")


class OctreeRow:
	def __init__ (self):
		self.blockID = 0
		self.minX = 0
		self.maxX = 0
		self.minY = 0
		self.maxY = 0
		self.minZ = 0
		self.maxZ = 0
		self.numParticles = 0
		self.offsetInFile = 0
		self.partitionLocation = 0

	def printMe(self):
		print(self.blockID, self.minX, self.maxX, self.minY, self.maxY, self.minZ, self.maxZ, self.numParticles, self.offsetInFile, self.partitionLocation)

class Octree:
	def __init__ (self):
		self.preShuffled = 0
		self.decompositionType = 0
		self.decompositionLevel = 0
		self.numEntries = 0
		self.rows = []

	def printMe(self):
		print("Pre-Shuffled:", self.preShuffled)
		print("Decomposition level:", self.decompositionType)
		print("Decomposition level:", self.decompositionLevel)
		print("Num Entries:", self.numEntries)

		print("(index, minX, maxX, minY, maxY, minZ,maxZ, #particles, offset_in_rank_file, rank_location)")
		for i in range(0,self.numEntries):
			self.rows[i].printMe()



def main(argv):

	# Read in arguments
	numArgs = len(sys.argv)

	if numArgs != 2:
		print ("An input file is required")
		exit()

	filename = sys.argv[1]




	# Open file
	with open(filename, mode='rb') as file:
		fileContent = file.read()


	# Read main header
	pos = 0
	headerInfo = GenericIOHeader()

	headerInfo.filetype = struct.unpack("8s", fileContent[pos:pos+8])[0]	
	pos = pos+8
		

	headerInfo.headerSize = struct.unpack("q", fileContent[pos:pos+8])[0]
	pos = pos + 8

	headerInfo.numElems = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8


	headerInfo.dims.append( struct.unpack("q", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.dims.append( struct.unpack("q", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.dims.append( struct.unpack("q", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.numVars = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	headerInfo.varSize = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	headerInfo.varStart = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	headerInfo.numRanks = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	headerInfo.rankSize = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	headerInfo.rankStart = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	headerInfo.globalHeaderSize = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8


	headerInfo.physOrigin.append( struct.unpack("d", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.physOrigin.append( struct.unpack("d", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.physOrigin.append( struct.unpack("d", fileContent[pos:pos+8])[0] )
	pos = pos + 8


	headerInfo.physScale.append( struct.unpack("d", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.physScale.append( struct.unpack("d", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.physScale.append( struct.unpack("d", fileContent[pos:pos+8])[0] )
	pos = pos + 8

	headerInfo.blockSize =struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	headerInfo.blockStart = struct.unpack("q", fileContent[pos:pos+8])[0] 
	pos = pos + 8

	if headerInfo.globalHeaderSize == 184:
		headerInfo.octreeSize = struct.unpack("q", fileContent[pos:pos+8])[0] 
		pos = pos + 8

		headerInfo.octreeStart = struct.unpack("q", fileContent[pos:pos+8])[0] 
		pos = pos + 8
	else:
		headerInfo.octreeSize = 0
		headerInfo.octreeStart = 0

	headerInfo.printMe()

	#print("\n")
	print "pos: ", pos 


	# Octree
	if headerInfo.octreeSize != 0:
		octreeData = Octree()

		octreeData.preShuffled = struct.unpack("q", fileContent[pos:pos+8])[0]
		pos = pos + 8

		octreeData.decompositionLevel = struct.unpack("q", fileContent[pos:pos+8])[0]
		pos = pos + 8

		octreeData.numEntries = struct.unpack("q", fileContent[pos:pos+8])[0]
		pos = pos + 8

		
		for i in range(0, octreeData.numEntries):
			octreeDataRow = OctreeRow()

			octreeDataRow.blockID = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.minX = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.maxX = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.minY = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.maxY = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.minZ = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.maxZ = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.numParticles = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.offsetInFile = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeDataRow.partitionLocation = struct.unpack("q", fileContent[pos:pos+8])[0]
			pos = pos + 8

			octreeData.rows.append(octreeDataRow)
		
		print("\nOctree:")
		octreeData.printMe()
		



	# Read variable info
	variableList = []
	numVars  = 0
	isHeader = False;
	print("\n\nVariables:")
	positionInFile = headerInfo.varStart
	

	#print "\nheaderInfo.varStart: ", headerInfo.varStart 
 
	#while positionInFile < headerInfo.rankStart:
	for var in range(0, headerInfo.numVars):
		_varInfo = GenericIOVariableInfo()

		indexInFile = positionInFile

		# Variable
		_temp = str( struct.unpack("256s", fileContent[indexInFile:indexInFile+256])[0] )
		_varInfo.varname = int( struct.unpack("q", fileContent[indexInFile:indexInFile+8])[0] )
		_varInfo.varname = _temp.replace('\x00','')


		if not isHeader:
			if _varInfo.varname == "$partition":
				isHeader = True

		#Flags
		indexInFile = indexInFile + 256
		_flags = int( struct.unpack("q", fileContent[indexInFile:indexInFile+8])[0] )
		_varInfo.isFloat  = (GenericIOVariableInfo.floatMask  & _flags) != 0
		_varInfo.isSigned = (GenericIOVariableInfo.signedMask & _flags) != 0
		_varInfo.isPhysCoordX = (GenericIOVariableInfo.phyXMask & _flags) != 0
		_varInfo.isPhysCoordY = (GenericIOVariableInfo.phyYMask & _flags) != 0
		_varInfo.isPhysCoordZ = (GenericIOVariableInfo.phyZMask & _flags) != 0
		_varInfo.maybePhysGhost = (GenericIOVariableInfo.ghostMask  & _flags) != 0


		#Size
		indexInFile = indexInFile + 8
		_varInfo.varSize = int( struct.unpack("q", fileContent[indexInFile:indexInFile+8])[0] )


		

		positionInFile = positionInFile + headerInfo.varSize

		_varInfo.printMe()
		variableList.append(_varInfo)
		numVars = numVars + 1


		#print "\npositionInFile: ", positionInFile 


	# Read rank info
	dataStart = 0
	numDataRows = 0
	_count = 0

	print("Read rank info:")
	print("(Coord X, Coord Y, Coord Z, #, Data Start row, partition id)")


	while positionInFile < headerInfo.headerSize:
		partitionX = struct.unpack("q", fileContent[positionInFile:positionInFile+8])[0]
		positionInFile = positionInFile + 8

		partitionY = struct.unpack("q", fileContent[positionInFile:positionInFile+8])[0]
		positionInFile = positionInFile + 8

		partitionZ = struct.unpack("q", fileContent[positionInFile:positionInFile+8])[0]
		positionInFile = positionInFile + 8

		numDataRows = struct.unpack("q", fileContent[positionInFile:positionInFile+8])[0]
		positionInFile = positionInFile + 8

		dataStart = struct.unpack("q", fileContent[positionInFile:positionInFile+8])[0]
		positionInFile = positionInFile + 8

		partitionID = struct.unpack("q", fileContent[positionInFile:positionInFile+8])[0]
		positionInFile = positionInFile + 8

		print (partitionX, partitionY, partitionZ, _count, dataStart, partitionID)

		_count = _count + 1

		#print "\npositionInFile: ", positionInFile 


	#print "\ndataStart: ", dataStart 

	if isHeader:
		dataList = []
		positionInFile = dataStart
		if _count == 1:
			positionInFile = dataStart
			print("\nMeta file info:")
			print("(partition id, In rank?, Coord X, Coord Y, Coord Z)")
			for _var in range(numVars):
				for _row in range(numDataRows):
					dataList.append( struct.unpack("i", fileContent[positionInFile:positionInFile+4])[0] )
					positionInFile = positionInFile + 4

				positionInFile = positionInFile + 8

			for _row in range(numDataRows):
				print(dataList[_row],dataList[numDataRows*1 + _row],dataList[numDataRows*2 + _row],dataList[numDataRows*3 + _row],dataList[numDataRows*4 + _row])

	else:
		print("\n")
		print("The rest is data...")



	# Read in data
if __name__ == "__main__":
	main(sys.argv[1:])


# python readHeader.py m000.full.mpicosmo.499