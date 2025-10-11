
cmake_minimum_required (VERSION 4.0)

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
	GIT_TAG        1.0.1
	EXCLUDE_FROM_ALL
)

#fmt
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        7.1.3
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/fmt
	EXCLUDE_FROM_ALL
)

#rapidxml
FetchContent_Declare(
	rapidxml
	GIT_REPOSITORY https://github.com/discord/rapidxml.git
	GIT_TAG        2ae4b2888165a393dfb6382168825fddf00c27b9
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/rapidxml
	EXCLUDE_FROM_ALL
)

#json
FetchContent_Declare(
	json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG        55f93686c01528224f448c19128836e7df245f72
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/json
	EXCLUDE_FROM_ALL
)

# imgui
FetchContent_Declare(
	imgui
	GIT_REPOSITORY	https://github.com/ocornut/imgui.git
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/imgui
	GIT_TAG 		v1.91.8-docking
	EXCLUDE_FROM_ALL
)

# implot
FetchContent_Declare(
	implot
	GIT_REPOSITORY	https://github.com/epezent/implot.git
    SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/implot
	EXCLUDE_FROM_ALL
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
	EXCLUDE_FROM_ALL
)
FetchContent_Declare(
	magic_enum
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/magic_enum
	GIT_REPOSITORY	https://github.com/Neargye/magic_enum.git
	GIT_TAG			v0.9.7
	EXCLUDE_FROM_ALL
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
	set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "")
	FetchContent_Declare(
		zlib
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/zlib
		GIT_REPOSITORY	https://github.com/madler/zlib.git
		GIT_TAG			v1.3
		GIT_SHALLOW 	TRUE
		OVERRIDE_FIND_PACKAGE
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(zlib)
	add_library(ZLIB::ZLIB ALIAS zlibstatic)

	#libpng
	set(PNG_SHARED 			OFF CACHE BOOL "" FORCE)
	set(PNG_STATIC 			ON 	CACHE BOOL "" FORCE)
	set(PNG_TESTS 			OFF CACHE BOOL "" FORCE)
	set(PNG_BUILD_ZLIB 		OFF	CACHE BOOL "" FORCE)
	set(PNG_TOOLS           OFF CACHE BOOL "" FORCE)
	set(PNG_TESTS 			OFF CACHE BOOL "" FORCE)
	set(SKIP_INSTALL_ALL 	ON 	CACHE BOOL "" FORCE)

	FetchContent_Declare(
		libpng
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libpng
		GIT_REPOSITORY	https://github.com/pnggroup/libpng.git
		GIT_TAG			v1.6.50
		GIT_SHALLOW 	TRUE
		EXCLUDE_FROM_ALL
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
		EXCLUDE_FROM_ALL
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
		GIT_TAG			main
		GIT_SHALLOW 	TRUE
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(sdl)



	set(SDLIMAGE_VENDORED OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_AVIF OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_BMP  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_GIF  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_JPG  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_JXL  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_LBM  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_PCX  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_PNG  ON CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_PNM  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_QOI  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_SVG  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_TGA  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_TIF  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_WEBP OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_XCF  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_XPM  OFF CACHE INTERNAL "" FORCE)
	set(SDLIMAGE_XV   OFF CACHE INTERNAL "" FORCE)
	FetchContent_Declare(
		sdl_image
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl_image
		GIT_REPOSITORY	https://github.com/libsdl-org/SDL_image.git
		GIT_TAG			release-3.2.4
		GIT_SHALLOW 	TRUE
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(sdl_image)

	set(SDLMIXER_FLAC_LIBFLAC      OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_FLAC_DRFLAC       OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_GME               OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_GME_SHARED        OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_MOD_XMP           OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_MP3_DRMP3         OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_MP3_MPG123        OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_MIDI_TIMIDITY     OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_OPUS              OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_VORBIS_STB        OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE INTERNAL "" FORCE)
	set(SDLMIXER_VORBIS_TREMOR     OFF CACHE INTERNAL "" FORCE)
	FetchContent_Declare(
		sdl_mixer
		SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/sdl_mixer
		GIT_REPOSITORY	https://github.com/libsdl-org/SDL_mixer.git
		GIT_TAG			93685a9006952fdc49c58fa0f95306a9cff1ed83
		GIT_SHALLOW 	TRUE
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(sdl_mixer)
endif()

#freetype
set(FT_DISABLE_HARFBUZZ ON CACHE INTERNAL "")
FetchContent_Declare(
	freetype
	URL https://download.savannah.gnu.org/releases/freetype/freetype-2.11.1.tar.xz
	SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/freetype
	EXCLUDE_FROM_ALL
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
		zlibstatic
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

)
if(NOT EMSCRIPTEN)
	target_link_libraries(fastfall
		PUBLIC
			SDL3::SDL3
			SDL3_image::SDL3_image
			SDL3_mixer::SDL3_mixer
			zlibstatic
			glew_s
	)
endif()