cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(LASvalidate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# defines targets and sources
add_subdirectory(src)

# contains LASread and LASzip libraries
add_subdirectory(LASread)