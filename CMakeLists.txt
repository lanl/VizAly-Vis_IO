cmake_minimum_required(VERSION 3.3)

project(VizAly)


# Subprojects
option(ENABLE_GioMPIQuery "Testing Querying of GenericIO files" OFF)
if (ENABLE_GioMPIQuery)
	add_subdirectory(GioMPIQuery)
endif()

