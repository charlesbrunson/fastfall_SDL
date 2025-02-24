
cmake_minimum_required (VERSION 3.18)

include(FetchContent)


set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(OpenGL_GL_PREFERENCE "GLVND" CACHE INTERNAL "")

cmake_policy(SET CMP0135 NEW)

# glm
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	#GIT_TAG        6ad79aae3eb5bf809c30bf1168171e9e55857e45
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

#fmt
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        7.1.3
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/fmt
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
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/imgui
	GIT_TAG 		v1.91.8-docking
)

# implot
FetchContent_Declare(
	implot
	GIT_REPOSITORY	https://github.com/epezent/implot.git
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/implot
)

# tracy
if (NOT ${FF_TRACE})
	set(TRACY_ENABLE OFF CACHE INTERNAL "Disable Tracy")
endif()

FetchContent_Declare(
	tracy
	GIT_REPOSITORY	https://github.com/wolfpld/tracy.git
	GIT_TAG			v0.10
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/tracy
	GIT_SHALLOW 	TRUE
	GIT_PROGRESS 	TRUE
)
FetchContent_Declare(
	magic_enum
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/magic_enum
	GIT_REPOSITORY	https://github.com/Neargye/magic_enum.git
	GIT_TAG			v0.9.7
)

FetchContent_MakeAvailable(
	glm
	fmt	
	json
	rapidxml
	imgui
	implot
	tracy
	magic_enum
)
target_compile_definitions(glm PUBLIC GLM_ENABLE_EXPERIMENTAL)

# compile imgui and implot as library
add_library(imgui STATIC
		imgui/imgui.cpp
		imgui/imgui_demo.cpp
		imgui/imgui_draw.cpp
		imgui/imgui_tables.cpp
		imgui/imgui_widgets.cpp
		imgui/backends/imgui_impl_sdl3.cpp
		imgui/backends/imgui_impl_opengl3.cpp

		implot/implot.cpp
		implot/implot_demo.cpp
		implot/implot_items.cpp
		)

target_include_directories(imgui PRIVATE
		imgui
		implot
		sdl/include
		)

target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW )



if (NOT EMSCRIPTEN)


	#zlib
	FetchContent_Declare(
		zlib
		URL			   https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.gz
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/zlib
	)
	FetchContent_MakeAvailable(zlib)

	if (MSVC)
		set(ZLIB_LIBRARY zlibstatic)
	else()
		set(ZLIB_LIBRARY zlib)
	endif()

	set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR})
	set(ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

	include_directories(
			${ZLIB_INCLUDE_DIRS}
	)
	configure_file(
			"${zlib_BINARY_DIR}/zconf.h"
			"${zlib_SOURCE_DIR}/zconf.h"
			COPYONLY
	)

	#libpng
	set(PNG_SHARED 			OFF CACHE BOOL "" FORCE)
	set(PNG_STATIC 			ON 	CACHE BOOL "" FORCE)
	set(PNG_TESTS 			OFF CACHE BOOL "" FORCE)
	set(PNG_BUILD_ZLIB 		ON 	CACHE BOOL "" FORCE)
	set(PNG_EXECUTABLES 	OFF CACHE BOOL "" FORCE)
	set(PNG_TESTS 			OFF CACHE BOOL "" FORCE)
	set(SKIP_INSTALL_ALL 	ON 	CACHE BOOL "" FORCE)

	FetchContent_Declare(
			libpng
			URL			https://github.com/glennrp/libpng/archive/refs/tags/v1.6.39.zip
			SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libpng
	)
	FetchContent_MakeAvailable(libpng)
	set(PNG_LIBRARY png_static)
	set(PNG_PNG_INCLUDE_DIR ${libpng_SOURCE_DIR})
	include_directories(${libpng_BINARY_DIR})

	# glew
	set(BUILD_UTILS OFF PARENT)
	FetchContent_Declare(
			glew
			URL				https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip
			SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/glew
			SOURCE_SUBDIR  build/cmake
	)
	FetchContent_MakeAvailable(glew)

	# sdl
	set(SDL_SHARED OFF CACHE INTERNAL "")
	set(SDL_STATIC ON  CACHE INTERNAL "")

	if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		# no idea why this is now needed on clion
		set(LIBC ON CACHE INTERNAL "")
	endif()

	FetchContent_Declare(
			sdl
			SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl
			GIT_REPOSITORY	https://github.com/libsdl-org/SDL.git
			GIT_TAG			release-3.2.4
			GIT_SHALLOW 	TRUE
			GIT_PROGRESS 	TRUE
	)
	FetchContent_MakeAvailable(sdl)

	set(SDLIMAGE_AVIF OFF  CACHE INTERNAL "")
	FetchContent_Declare(
			sdl_image
			SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl_image
			GIT_REPOSITORY	https://github.com/libsdl-org/SDL_image.git
			GIT_TAG			release-3.2.0
			GIT_SHALLOW 	TRUE
			GIT_PROGRESS 	TRUE
	)
	FetchContent_MakeAvailable(sdl_image)

	FetchContent_Declare(
		miniaudio
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio
		GIT_REPOSITORY	https://github.com/mackron/miniaudio.git
		GIT_TAG			0.11.21
		GIT_SHALLOW 	TRUE
		GIT_PROGRESS 	TRUE
	)
	FetchContent_MakeAvailable(miniaudio)

	add_library(miniaudio INTERFACE ${miniaudio_SOURCE_DIR}/miniaudio.h)
	add_library(miniaudio::miniaudio ALIAS miniaudio)
	target_include_directories(miniaudio INTERFACE ${miniaudio_SOURCE_DIR})

endif()

#freetype
#set(ZLIB_LIBRARY zlib)
#set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE INTERNAL "")
set(FT_DISABLE_HARFBUZZ ON CACHE INTERNAL "")
FetchContent_Declare(
		freetype
		URL https://download.savannah.gnu.org/releases/freetype/freetype-2.11.1.tar.xz
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/freetype
)
FetchContent_MakeAvailable(freetype)

if (EMSCRIPTEN)
	target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_ES3)

	target_compile_options(imgui PRIVATE -sUSE_SDL=3)
	target_link_options(imgui PRIVATE -sUSE_SDL=3)

	target_compile_options(freetype PRIVATE -sUSE_LIBPNG=1)
	target_link_options(freetype PRIVATE -sUSE_LIBPNG=1)
endif()

# add all include directories to fastfall
target_include_directories(fastfall
PUBLIC
	glm
	imgui
	implot
	freetype/include
	rapidxml
	json/include
	fmt/include
	tracy/public/
)


if(NOT EMSCRIPTEN)
	target_include_directories(fastfall
	PUBLIC
		glew/include
		sdl_image/include
		sdl/include
		libpng
		zlib
	)
	include_directories(
		zlib
	)
endif()


# add all libraries and includes to fastfall
target_link_libraries(fastfall
	PUBLIC
		glm
		fmt::fmt
		imgui
		freetype
		Tracy::TracyClient
		magic_enum::magic_enum
		miniaudio::miniaudio
)
if(NOT EMSCRIPTEN)
	target_link_libraries(fastfall
		PUBLIC
			SDL3::SDL3
			SDL3_image::SDL3_image
			zlibstatic
			glew_s
	)
endif()