# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.19)

project ("fastfall_SDL")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

#zlib
FetchContent_Declare(
	zlib
	URL			https://zlib.net/zlib-1.2.11.tar.gz
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/zlib
)

#libpng
FetchContent_Declare(
	libpng
	URL			https://download.sourceforge.net/libpng/libpng-1.6.37.tar.gz
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/libpng
)

# sdl
FetchContent_Declare(
	sdl
	URL			https://www.libsdl.org/release/SDL2-2.0.14.tar.gz
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/sdl
)

# sdl_image
FetchContent_Declare(
	sdl_image
	URL			https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/sdl_image
)

# glew
FetchContent_Declare(
	glew
	URL			https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.tgz/download
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/glew
	SOURCE_SUBDIR  build/cmake
)

# glm
FetchContent_Declare(
	glm
	URL			https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/glm
)

# fmt
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        7.1.3
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/fmt
)

# flatbuffers
set(FLATBUFFERS_BUILD_TESTS OFF)
FetchContent_Declare(
	flatbuffers
	GIT_REPOSITORY https://github.com/google/flatbuffers.git
	GIT_TAG        v1.12.0
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/flatbuffers
)

# rapidxml
FetchContent_Declare(
	rapidxml
	GIT_REPOSITORY https://github.com/discord/rapidxml.git
	GIT_TAG        2ae4b2888165a393dfb6382168825fddf00c27b9
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/external/rapidxml
)

# imgui
FetchContent_Declare(
	imgui
	GIT_REPOSITORY	https://github.com/ocornut/imgui.git
	GIT_TAG			docking # docking branch
	GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
    SOURCE_DIR		${CMAKE_SOURCE_DIR}/external/imgui
)

FetchContent_MakeAvailable(
	zlib
)
set(ZLIB_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/external/zlib)
if (EXISTS ${zlib_SOURCE_DIR}/zconf.h.included)
	file(RENAME ${zlib_SOURCE_DIR}/zconf.h.included ${zlib_SOURCE_DIR}/zconf.h)
endif()

FetchContent_MakeAvailable(
	libpng
	sdl
	sdl_image
	glew
	glm
	fmt
	flatbuffers
	rapidxml
	imgui
)

# build imgui as library
file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h)
add_library(imgui STATIC 
	${IMGUI_SOURCES} 
	${IMGUI_HEADERS} 
	${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h
	${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
	)
target_include_directories(imgui PRIVATE "${imgui_SOURCE_DIR}")
target_include_directories(imgui PRIVATE "${sdl_SOURCE_DIR}/include")
target_include_directories(imgui PRIVATE "${glew_SOURCE_DIR}/include")
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

#build sdl_image as library
file(GLOB SDLIMG_SOURCES ${sdl_image_SOURCE_DIR}/*.c)
file(GLOB SDLIMG_HEARS ${sdl_image_SOURCE_DIR}/*.h)
add_library(SDL2image STATIC 
	${SDLIMG_SOURCES} 
	${SDLIMG_HEARS} 
	)
target_include_directories(SDL2image PRIVATE "${sdl_SOURCE_DIR}/include")
target_include_directories(SDL2image PRIVATE "${sdl_image_SOURCE_DIR}")
target_include_directories(SDL2image PRIVATE "${libpng_SOURCE_DIR}")
target_link_libraries(SDL2image PRIVATE png_static)
target_compile_definitions(SDL2image PUBLIC LOAD_PNG LOAD_BMP)

# Include sub-projects.
add_subdirectory ("fastfall_SDL")
