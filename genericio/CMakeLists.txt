cmake_minimum_required(VERSION 3.3)

project(GenericIO)

set(CMAKE_CXX_STANDARD 11)

# Create directories
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/build/frontend)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/build/mpi)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build//frontend)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/frontend)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/mpi)


set(PROJECT_FRONTEND_FILES
	GenericIOPrint.cxx 
	GenericIOVerify.cxx
	GenericIOFileInfo.cxx
)

set(PROJECT_MPI_FILES
	GenericIOPrint.cxx 
	GenericIOVerify.cxx
	GenericIOBenchmarkRead.cxx
	GenericIOBenchmarkWrite.cxx
	GenericIORewrite.cxx
)

# MPI
find_package(MPI)
if (MPI_FOUND)
	include_directories(${MPI_INCLUDE_PATH}/*)
	SET(CMAKE_CXX_COMPILER mpicxx)
	set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
	set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
else()
	message(FATAL_ERROR "Could not find any MPI installation!!!") 
endif()

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/blosc)

#Generate the shared library from the sources
add_library(GenericIO SHARED GenericIO.cxx)

#add_executable(GenericIOPrint GenericIOPrint.cxx)
#target_link_libraries(GenericIOPrint ${MPI_LIBRARIES} ${VTK_LIBRARIES})