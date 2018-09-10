import sys, os
from md5Utils import *


def runTest(filename, numTests, successCount):
	numTests = numTests + 1
	passed = runMD5FileTest(filename)
	successCount = successCount + passed
	printTestResult(filename, passed)
	return (numTests, successCount)


def main():
	print ("Running tests ...")

	# Run Serial tests
	bashCommand = "./runSerialTests"
	os.system(bashCommand)
	

	# Run MPI Tests
	bashCommand = "mpirun -np 4 ./runMPITests 10"
	os.system(bashCommand)

	# Run Comparison
	numTests = 0
	successCount = 0
	dirs = os.listdir( "." )
	for file in dirs:
		if file.endswith(".vtu") or file.endswith(".pvtu"):
			numTests, successCount = runTest(file, numTests, successCount)
   		elif file.endswith(".vts") or file.endswith(".pvts"):
   			numTests, successCount = runTest(file, numTests, successCount)
   		elif file.endswith(".vtr") or file.endswith(".pvtr"):
   			numTests, successCount = runTest(file, numTests, successCount)

   	# Cleanup
   	bashCommand = "rm -f *.vtu; rm -f *.vtr; rm -f *.vts; rm -f *.pvtu; rm -f *.pvts; rm -f *.pvtr"
	os.system(bashCommand)

	# Print output
	print ("============================================================")
	print ("Test run summary: ")
	print ("#tests run: %d, #passed: %d, #failed: %d" %(numTests, successCount, (numTests-successCount)))
	print ("============================================================")

if __name__ == '__main__':
	main()

# Usage
# python regressionTest.py 