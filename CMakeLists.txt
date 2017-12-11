cmake_minimum_required(VERSION 3.3)

project(VizAly)


# Subprojects
option(ENABLE_HelloWorld "Testing Subprojects" OFF)
if (ENABLE_HelloWorld)
	add_subdirectory(HelloWorld)
endif()


# Remove subprojects
add_custom_target(clean-subprojects
	COMMAND rm -rf HelloWorld
	)