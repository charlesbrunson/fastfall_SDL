# CMakeList.txt : CMake project for fastfall_SDL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.16)

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

add_custom_command(
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

set(FF_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/include


	${CMAKE_CURRENT_SOURCE_DIR}/external/rapidxml
	${CMAKE_CURRENT_SOURCE_DIR}/external/fmt/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/flatbuffers/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/glm
	
	${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/external/implot
)

set(FF_LIBRARIES
	glm
	fmt::fmt-header-only
	flatbuffers
)


if(NOT EMSCRIPTEN)

	set(FF_INCLUDES
		${FF_INCLUDES}
		${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include
		${CMAKE_CURRENT_SOURCE_DIR}/external/sdl_image
		${CMAKE_CURRENT_SOURCE_DIR}/external/sdl/include
	)

	set(FF_LIBRARIES 
		${FF_LIBRARIES}
		zlibstatic
		SDL2image 
		glew_s
		SDL2main
		SDL2-static
	)
endif()


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

set(FF_SOURCES 
	${FF_SOURCES} 
	${IMGUI_SOURCES}
	${IMPLOT_SOURCES}
	${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h
	${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

set(FF_HEADERS 
	${FF_HEADERS} 
	${IMGUI_HEADERS} 
	${IMPLOT_HEADERS} 
)

add_library (fastfall STATIC ${FF_SOURCES} ${FF_HEADERS} "include/fastfall/game/trigger/Trigger.hpp")


if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	target_compile_options(fastfall PUBLIC -Wno-deprecated-volatile)
	if(NOT EMSCRIPTEN)
		target_compile_options(fastfall PUBLIC -stdlib=libc++)
		target_link_options(fastfall PUBLIC -lz)
		target_link_options(fastfall PUBLIC -lc++)
	else()
		set(CMAKE_EXECUTABLE_SUFFIX .html)
		set(USE_FLAGS "-s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"bmp\",\"png\"]' -s FULL_ES3=1 -s WASM=1 -s OFFSCREEN_FRAMEBUFFER=1 -g --preload-file ../data@/data --preload-file ../gamecontrollerdb.txt@/gamecontrollerdb.txt -s ALLOW_MEMORY_GROWTH=1")
		set_target_properties(fastfall PROPERTIES LINK_FLAGS "${USE_FLAGS} --emrun")
		set_target_properties(fastfall PROPERTIES COMPILE_FLAGS ${USE_FLAGS})
	endif()
endif()

# depends on compiled flatbuffers headers
add_dependencies(fastfall fbs_compiled)

# includes
target_include_directories(fastfall PUBLIC ${FF_INCLUDES})

# libraries
target_link_libraries(fastfall PUBLIC ${FF_LIBRARIES})

# compile definitions
target_compile_definitions(fastfall PUBLIC
	IMGUI_IMPL_OPENGL_LOADER_GLEW
)

target_compile_definitions(fastfall PRIVATE
	DEBUG
)

if(NOT EMSCRIPTEN)
	target_compile_definitions(fastfall PRIVATE
		FF_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../"
	)
else()
	target_compile_definitions(fastfall PUBLIC
		IMGUI_IMPL_OPENGL_ES3
	)
	target_compile_definitions(fastfall PRIVATE
		FF_DATA_DIR=""
	)
endif()
