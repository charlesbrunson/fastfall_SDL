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
file(GLOB FLATC_FBS "${SCHEMA_DIR}/*.fbs")

foreach(fbs IN ITEMS ${FLATC_FBS}) 
	string(REGEX REPLACE "[.]fbs" "-flat.hpp" out ${fbs})
	list(APPEND FLATC_HPP ${out})
endforeach()

add_custom_command(#fbs_compiled
	COMMAND ${FLATC} 
		--cpp 
		-o "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_DIR}/" 
		--filename-suffix "-flat"
		--filename-ext "hpp"
		${FLATC_FBS}
	OUTPUT
		${FLATC_HPP}
	DEPENDS
		${FLATC_FBS}
)

add_custom_target(fbs_compiled
	DEPENDS
		${FLATC_HPP}
)

# grab fastfall source
file(GLOB_RECURSE FF_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)
file(GLOB_RECURSE FF_HEADERS 
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

# grab imgui source
set(imgui_SOURCE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
set(implot_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/implot")

file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h)
file(GLOB IMPLOT_SOURCES ${implot_SOURCE_DIR}/*.cpp)
file(GLOB IMPLOT_HEADERS ${implot_SOURCE_DIR}/*.h)

# declare fastfall library
add_library (fastfall STATIC
	${FF_SOURCES}
	${FF_HEADERS}

	# imgui stuff
	
	${IMGUI_SOURCES} 
	${IMPLOT_SOURCES} 
	${IMGUI_HEADERS} 
	${IMPLOT_HEADERS} 
	${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h
	${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# depends on compiled flatbuffers headers
add_dependencies(fastfall fbs_compiled)

# includes
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

# compile definitions
target_compile_definitions(fastfall PRIVATE
	#GLEW_STATIC
	IMGUI_IMPL_OPENGL_LOADER_GLEW
	DEBUG
)
target_compile_definitions(fastfall PRIVATE
	FF_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../"
)

# libraries
target_link_libraries(fastfall PUBLIC 
	glm
	fmt::fmt-header-only
	flatbuffers
	zlibstatic
	SDL2main
	SDL2-static
	glew_s
	SDL2image 
	#libimgui 
)

#target_compile_options(fastfall PUBLIC -stdlib=libc++)
#target_compile_options(fastfall PUBLIC -Wno-deprecated-volatile)
#target_link_options(fastfall PUBLIC -lz)
#target_link_options(fastfall PUBLIC -lc++)
