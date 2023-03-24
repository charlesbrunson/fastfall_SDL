# CMakeList.txt : CMake project for fastfall_SDL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.16)

add_library(fastfall STATIC ${FF_SOURCES})
add_subdirectory("external")
add_subdirectory("src")
set_target_properties(fastfall PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

target_include_directories(fastfall PUBLIC  include)
target_include_directories(fastfall PRIVATE src)

message(STATUS "FASTFALL COMPILER   -- ${CMAKE_CXX_COMPILER_ID}")

if(EMSCRIPTEN)
	if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
		message(STATUS "EMSCRIPTEN DEBUG")
		target_compile_options(fastfall PUBLIC -g -O0)
		target_link_options(fastfall PUBLIC -g -O0 -sDEMANGLE_SUPPORT=1)
	endif()

	target_compile_options(fastfall PUBLIC 
		-sUSE_ZLIB=1 
		-sUSE_LIBPNG=1 
		-sUSE_SDL=2 
		-sUSE_SDL_IMAGE=2 
		-sSDL2_IMAGE_FORMATS=["bmp","png"]	
	)

	target_link_options(fastfall PUBLIC 
		-sUSE_ZLIB=1 
		-sUSE_LIBPNG=1 
		-sUSE_SDL=2 
		-sUSE_SDL_IMAGE=2 
		-sSDL2_IMAGE_FORMATS=["bmp","png"]	
		-sWASM=1
		-sWASM_BIGINT=1 
		-sFULL_ES3=1 
		-sALLOW_MEMORY_GROWTH=1 
	)

	if (${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_compile_options(fastfall PUBLIC -O3)
		target_link_options(fastfall PUBLIC -O3 -sBINARYEN_IGNORE_IMPLICIT_TRAPS=1)
	endif()
endif()

if(EMSCRIPTEN)
	message(STATUS "FASTFALL EMSCRIPTEN -- ON")
else()
	message(STATUS "FASTFALL EMSCRIPTEN -- OFF")
endif()

# compile definitions
target_compile_definitions(fastfall PUBLIC DEBUG)

if (WIN32)
	# thanks windows
	target_compile_definitions(fastfall PUBLIC NOMINMAX)
endif()

add_subdirectory("test")


