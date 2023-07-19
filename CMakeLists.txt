project(HelloWorld)
set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required(VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)


# SZ
find_library(SZ_LIBRARY ${SZ_LIBRARY} CACHE STRING "SZ library")
if (NOT SZ_LIBRARY)
    message(FATAL_ERROR "Could NOT find SZ")
endif()

set (SZ_INCLUDE_PATH ${SZ_INCLUDE_PATH} CACHE PATH "SZ include directory")
include_directories(${SZ_INCLUDE_PATH})



# BLOSC
find_library(BLOSC_LIBRARY ${BLOSC_LIBRARY} CACHE STRING "BLOSC_LIBRARY library")
if (NOT BLOSC_LIBRARY)
    message(FATAL_ERROR "Could NOT find BLOSC")
endif()

set (BLOSC_INCLUDE_PATH ${BLOSC_INCLUDE_PATH} CACHE PATH "BLOSC include directory")
include_directories(${BLOSC_INCLUDE_PATH})





# MPI
find_package(MPI)
if (MPI_FOUND)
	include_directories(${MPI_INCLUDE_PATH})
	if (${CBENCH_PLATFORM} MATCHES "Cori")
		set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
		set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
	else()
		set(CMAKE_CXX_COMPILE_FLAGS "${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS}")
		set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS}")
		set(CMAKE_C_COMPILER mpicc)
		set(CMAKE_CXX_COMPILER mpicxx)
	endif()
else()
	message(FATAL_ERROR "Could not find any MPI installation!!!") 
endif()


# GenericIO
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/genericio)
add_definitions(-DGENERICIO_NO_COMPRESSION)
add_library(GenericIO STATIC "${CMAKE_SOURCE_DIR}/thirdparty/genericio/GenericIO.cxx")
set_property(TARGET GenericIO PROPERTY POSITION_INDEPENDENT_CODE ON)
install(TARGETS GenericIO DESTINATION .)



add_executable(testC testCompress.cpp)

#target_compile_definitions(testC PUBLIC OCTREE_ON)
target_link_libraries(testC PRIVATE ${SZ_LIBRARY} ${BLOSC_LIBRARY} GenericIO)
