# CMakeList.txt : CMake project for fastfall_SDL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# build fastfall dependencies
add_subdirectory("external")


# generate schema files with flatc
set(FLATC "${CMAKE_CURRENT_SOURCE_DIR}/tools/flatc.exe")
set(SCHEMA_DIR "include/fastfall/schema")
file(GLOB FLATC_SCHEMAS "${SCHEMA_DIR}/*.fbs")
		 
file(GLOB_RECURSE FF_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)
file(GLOB_RECURSE FF_HEADERS 
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

add_library (fastfall STATIC
	${FF_SOURCES}
	${FF_HEADERS}
)

add_custom_target(fbs_compiled
	COMMAND ${FLATC} 
		--cpp 
		-o "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_DIR}/" 
		--filename-suffix "-flat"
		--filename-ext "hpp"
		${FLATC_SCHEMAS}
)
add_dependencies(fastfall fbs_compiled)

target_include_directories(fastfall PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include

	${CMAKE_CURRENT_SOURCE_DIR}/external/rapidxml
	${CMAKE_CURRENT_SOURCE_DIR}/external/fmt/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/flatbuffers/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/glm
	${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sdl/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sdl_image
	${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/external/implot
)

target_compile_definitions(fastfall PRIVATE
	#GLEW_STATIC
	DEBUG
)
target_link_libraries(fastfall PUBLIC 
	glm
	fmt
	flatbuffers
	zlibstatic
	SDL2-static
	SDL2main
	glew_s
	SDL2image 
	libimgui 
)

