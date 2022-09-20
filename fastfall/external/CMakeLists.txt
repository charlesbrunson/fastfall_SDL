
cmake_minimum_required (VERSION 3.18)

include(FetchContent)


set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(OpenGL_GL_PREFERENCE "GLVND" CACHE INTERNAL "")

# glm
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG        6ad79aae3eb5bf809c30bf1168171e9e55857e45
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

#fmt
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        7.1.3
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/fmt
)

#flatbuffers
set(FLATBUFFERS_BUILD_TESTS 	OFF CACHE INTERNAL "")
set(FLATBUFFERS_INSTALL 		OFF CACHE INTERNAL "")
set(FLATBUFFERS_BUILD_FLATC 	OFF CACHE INTERNAL "")
set(FLATBUFFERS_BUILD_FLATLIB 	ON  CACHE INTERNAL "")
set(FLATBUFFERS_BUILD_CPP17 	ON  CACHE INTERNAL "")
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

#json
FetchContent_Declare(
	json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG        a09bfa5faa00da25944b07df131e0f8338d2251e
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/json
)

# imgui
FetchContent_Declare(
	imgui
	GIT_REPOSITORY	https://github.com/ocornut/imgui.git
	#GIT_TAG			bac748fa95ac003c7b354139980f8b4b7f6ac5da # docking branch
	#GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
	GIT_TAG			v1.87
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

# implot
FetchContent_Declare(
	implot
	GIT_REPOSITORY	https://github.com/epezent/implot.git
	GIT_TAG			v0.13
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/implot
)


#freetype
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE INTERNAL "")
set(FT_DISABLE_HARFBUZZ ON CACHE INTERNAL "")
FetchContent_Declare(
	freetype
	URL https://download.savannah.gnu.org/releases/freetype/freetype-2.11.1.tar.xz
	SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/freetype
)

FetchContent_MakeAvailable(
	freetype
	glm
	fmt	
	flatbuffers
	json
	rapidxml
	imgui
	implot
)

if (EMSCRIPTEN)

	target_compile_options(freetype PRIVATE -s USE_LIBPNG=1)
	target_link_options(freetype PRIVATE -s USE_LIBPNG=1)

endif()


if (NOT EMSCRIPTEN)

	# sdl
	set(SDL_SHARED OFF CACHE INTERNAL "")
	set(SDL_STATIC ON  CACHE INTERNAL "")

	if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		# no idea why this is now needed on clion
		set(LIBC ON CACHE INTERNAL "")
	endif()

	FetchContent_Declare(
		sdl
		URL			https://www.libsdl.org/release/SDL2-2.0.14.tar.gz
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl
	)

	# glew
	set(BUILD_UTILS OFF PARENT)
	FetchContent_Declare(
		glew
		URL			https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.tgz/download
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/glew
		SOURCE_SUBDIR  build/cmake
	)

	#zlib
	FetchContent_Declare(
		zlib
		URL			http://zlib.net/fossils/zlib-1.2.12.tar.gz
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/zlib
	)


	#libpng
	set(PNG_SHARED OFF 		CACHE INTERNAL "")
	set(PNG_TESTS OFF 		CACHE INTERNAL "")
	set(PNG_BUILD_ZLIB ON 	CACHE INTERNAL "" )
	FetchContent_Declare(
		libpng
		URL			https://download.sourceforge.net/libpng/libpng-1.6.37.tar.gz
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libpng
	)

	# sdl
	FetchContent_Declare(
		sdl
		URL			https://www.libsdl.org/release/SDL2-2.0.10.tar.gz
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl
	)

	# sdl_image
	FetchContent_Declare(
		sdl_image
		URL			https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl_image
	)

	FetchContent_MakeAvailable(
		glew
		zlib
		libpng
		sdl
		sdl_image
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

endif()

