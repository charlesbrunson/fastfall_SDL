
cmake_minimum_required (VERSION 3.19)

include(FetchContent)

set(BUILD_STATIC_LIBS ON)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# glm
FetchContent_Declare(
	glm
	URL			https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/glm
)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	add_compile_options(-Wno-deprecated-volatile)
endif()

#fmt
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        7.1.3
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/fmt
)


#flatbuffers
set(FLATBUFFERS_BUILD_TESTS OFF)
set(FLATBUFFERS_INSTALL OFF)
set(FLATBUFFERS_BUILD_FLATLIB OFF)
set(FLATBUFFERS_BUILD_FLATC OFF)
set(FLATBUFFERS_BUILD_CPP17 OFF)
set(FLATBUFFERS_LIBCXX_WITH_CLANG OFF)
FetchContent_Declare(
	flatbuffers
	GIT_REPOSITORY https://github.com/google/flatbuffers.git
	GIT_TAG        v1.12.0
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/flatbuffers
)

#rapidxml
FetchContent_Declare(
	rapidxml
	GIT_REPOSITORY https://github.com/discord/rapidxml.git
	GIT_TAG        2ae4b2888165a393dfb6382168825fddf00c27b9
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/rapidxml
)

#zlib
FetchContent_Declare(
	zlib
	URL			https://zlib.net/zlib-1.2.11.tar.gz
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/zlib
)

#libpng
set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)
set(PNG_BUILD_ZLIB ON)
include_directories(${ZLIB_INCLUDE_DIR})
FetchContent_Declare(
	libpng
	URL			https://download.sourceforge.net/libpng/libpng-1.6.37.tar.gz
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libpng
)

# sdl
FetchContent_Declare(
	sdl
	URL			https://www.libsdl.org/release/SDL2-2.0.14.tar.gz
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl
)

# sdl_image
FetchContent_Declare(
	sdl_image
	URL			https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl_image
)

# glew
FetchContent_Declare(
	glew
	URL			https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.tgz/download
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/glew
	SOURCE_SUBDIR  build/cmake
)

# imgui
FetchContent_Declare(
	imgui
	GIT_REPOSITORY	https://github.com/ocornut/imgui.git
	GIT_TAG			docking # docking branch
	GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

# implot
FetchContent_Declare(
	implot
	GIT_REPOSITORY	https://github.com/epezent/implot.git
	GIT_TAG			v0.9
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/implot
)

FetchContent_MakeAvailable(
	glm
	fmt
	flatbuffers
	rapidxml
	zlib
	libpng
	sdl
	sdl_image
	glew
	imgui
	implot
)

include_directories(
	${zlib_BINARY_DIR}
	${libpng_BINARY_DIR}
)
configure_file(
	"${zlib_BINARY_DIR}/zconf.h"
	"${zlib_SOURCE_DIR}/zconf.h"
	COPYONLY
)
configure_file(
	"${libpng_BINARY_DIR}/pnglibconf.h"
	"${libpng_SOURCE_DIR}/pnglibconf.h"
	COPYONLY
)

# build sdl_image as library
file(GLOB SDLIMG_SOURCES ${sdl_image_SOURCE_DIR}/*.c)
file(GLOB SDLIMG_HEADERS ${sdl_image_SOURCE_DIR}/*.h)
add_library(SDL2image STATIC 
	${SDLIMG_SOURCES} 
	${SDLIMG_HEADERS} 
	)
target_include_directories(SDL2image PRIVATE "${sdl_SOURCE_DIR}/include")
target_include_directories(SDL2image PRIVATE "${sdl_image_SOURCE_DIR}")
target_include_directories(SDL2image PRIVATE "${libpng_SOURCE_DIR}")
target_link_libraries(SDL2image PUBLIC png_static)
target_compile_definitions(SDL2image PRIVATE LOAD_PNG LOAD_BMP)

# build flatbuffers as a library
add_library(flatbuffers STATIC 
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/base.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/flatbuffers.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/hash.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/idl.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/util.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/reflection.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/reflection_generated.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/stl_emulation.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/flexbuffers.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/registry.h
  ${flatbuffers_SOURCE_DIR}/include/flatbuffers/minireflect.h
  ${flatbuffers_SOURCE_DIR}/src/idl_parser.cpp
  ${flatbuffers_SOURCE_DIR}/src/idl_gen_text.cpp
  ${flatbuffers_SOURCE_DIR}/src/reflection.cpp
  ${flatbuffers_SOURCE_DIR}/src/util.cpp
)
target_include_directories(flatbuffers PUBLIC
    ${flatbuffers_SOURCE_DIR}/include
)
