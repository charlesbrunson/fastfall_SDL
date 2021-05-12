
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory ("external")

file(GLOB_RECURSE FF_UTIL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB_RECURSE FF_UTIL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*)

add_library(ff_util STATIC
	${FF_UTIL_SOURCES}
	${FF_UTIL_HEADERS}
)

set(FF_UTIL_INCLUDE	${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "fastfall util include paths")
set(FF_UTIL_LIBRARY ff_util								CACHE STRING "fastfall util library name")

target_include_directories(ff_util PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_include_directories(ff_util PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/external/rapidxml
	${CMAKE_CURRENT_SOURCE_DIR}/external/fmt/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/flatbuffers/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/glm
)

target_link_libraries(ff_util PRIVATE 
	glm 
	fmt
	flatbuffers
)