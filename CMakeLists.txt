cmake_minimum_required(VERSION 3.3)

project(VizAly)


# Subprojects
option(ENABLE_HelloWorld "Testing Subprojects" OFF)
if (ENABLE_HelloWorld)
	add_subdirectory(HelloWorld)
endif()


option(ENABLE_GenericIO "GenericIO" OFF)
if (ENABLE_GenericIO)
    add_subdirectory(genericio)
endif()


# Prevent in source builds since they are evil
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In source build is not supported, please restore modified
			 files, delete CMake generated things (CMakeFiles folder and 
			 CMakeCache.txt), and build out of source")	
endif()


# Remove subprojects
add_custom_target(clean-subprojects
	COMMAND rm -rf HelloWorld
	COMMAND rm -rf genericio
)
