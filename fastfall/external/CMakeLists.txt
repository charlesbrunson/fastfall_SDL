
cmake_minimum_required (VERSION 4.0)

# include(FetchContent)

include(cmake/get_cpm.cmake)

# est(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
# set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
# set(OpenGL_GL_PREFERENCE "GLVND" CACHE INTERNAL "")

# cmake_policy(SET CMP0135 NEW)

# glm

CPMAddPackage(
		URI "gh:g-truc/glm#1.0.2"
		OPTIONS
		"BUILD_STATIC_LIBS ON"
		"BUILD_SHARED_LIBS OFF"
		"GLM_ENABLE_CXX_20 ON"
		"OpenGL_GL_PREFERENCE GLVND"
)
target_compile_definitions(glm PUBLIC GLM_ENABLE_EXPERIMENTAL)

CPMAddPackage("gh:fmtlib/fmt#12.1.0")
CPMAddPackage("gh:CodeFinder2/rapidxml#v1.13")
CPMAddPackage(
	URI "gh:nlohmann/json#v3.12.0"
	OPTIONS
		"JSON_BuildTests OFF"
		"JSON_Install OFF"
		"JSON_MultipleHeaders ON"
)

CPMAddPackage("gh:ocornut/imgui#v1.92.4-docking")

CPMAddPackage(
	URI "gh:wolfpld/tracy#v0.12.2"
	OPTIONS
	"TRACY_ENABLE ${FF_TRACE}"
)

CPMAddPackage("gh:Neargye/magic_enum#v0.9.7")


CPMAddPackage("gh:epezent/implot#master")



if (NOT EMSCRIPTEN)

	CPMAddPackage(
		URI "gh:madler/zlib#v1.3.1"
		OPTIONS
			"ZLIB_BUILD_EXAMPLES OFF"
			"ZLIB_BUILD_TESTING OFF"
			"ZLIB_BUILD_SHARED OFF"
			"ZLIB_BUILD_STATIC ON"
			"ZLIB_INSTALL OFF"
			"ZLIB_PREFIX OFF"
	)
	add_library(ZLIB::ZLIB ALIAS zlibstatic)

	CPMAddPackage(
		URI "gh:pnggroup/libpng#v1.6.50"
		OPTIONS
			"SKIP_INSTALL_ALL ON"
			"PNG_SHARED OFF"
			"PNG_STATIC ON"
			"PNG_TESTS OFF"
			"PNG_TOOLS OFF"
			"PNG_TESTS OFF"
			"ZLIB_FOUND ${zlib_ADDED}"
			"ZLIB_ROOT ${zlib_SOURCE_DIR}"
			"ZLIB_LIBRARY ZLIB::ZLIB"
			"ZLIB_LIBRARIES ZLIB::ZLIB"
			"ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR};${zlib_BINARY_DIR}"
			"ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR};${zlib_BINARY_DIR}"
			"ZLIB_USE_STATIC_LIBS ON"
	)

	CPMAddPackage(
		URI "gh:Perlmint/glew-cmake#glew-cmake-2.2.0"
		OPTIONS
			"OpenGL_GL_PREFERENCE GLVND"
			"glew-cmake_BUILD_SHARED OFF"
			"ONLY_LIBS ON"
	)

	CPMAddPackage(
		URI "gh:libsdl-org/SDL#preview-3.3.2"
		OPTIONS
			"SDL_SHARED OFF"
			"SDL_STATIC ON"
			"SDL_TEST_LIBRARY OFF"
	)
	set(SDL3_DIR ${SDL_BINARY_DIR})

	CPMAddPackage(
		URI "gh:libsdl-org/SDL_image#main"
		OPTIONS
			"BUILD_SHARED_LIBS OFF"
			"SDLIMAGE_INSTALL OFF"
			"SDLIMAGE_VENDORED OFF"
			"SDLIMAGE_AVIF OFF"
			"SDLIMAGE_BMP OFF"
			"SDLIMAGE_GIF OFF"
			"SDLIMAGE_JPG OFF"
			"SDLIMAGE_JXL OFF"
			"SDLIMAGE_LBM OFF"
			"SDLIMAGE_PCX OFF"
			"SDLIMAGE_PNG ON"
			"SDLIMAGE_PNM OFF"
			"SDLIMAGE_QOI OFF"
			"SDLIMAGE_SVG OFF"
			"SDLIMAGE_TGA OFF"
			"SDLIMAGE_TIF OFF"
			"SDLIMAGE_WEBP OFF"
			"SDLIMAGE_XCF OFF"
			"SDLIMAGE_XPM OFF"
			"SDLIMAGE_XV OFF"
			"SDLIMAGE_STB OFF"
	)

	CPMAddPackage(
		URI "gh:libsdl-org/SDL_mixer#main"
		OPTIONS
			"SDLMIXER_AIFF OFF"
			"SDLMIXER_VOC OFF"
			"SDLMIXER_AU OFF"
			"SDLMIXER_FLAC_LIBFLAC OFF"
			"SDLMIXER_FLAC_DRFLAC OFF"
			"SDLMIXER_GME OFF"
			"SDLMIXER_GME_SHARED OFF"
			"SDLMIXER_MOD_XMP OFF"
			"SDLMIXER_MP3_DRMP3 OFF"
			"SDLMIXER_MP3_MPG123 OFF"
			"SDLMIXER_MIDI_TIMIDITY OFF"
			"SDLMIXER_OPUS OFF"
			"SDLMIXER_VORBIS_STB OFF"
			"SDLMIXER_VORBIS_VORBISFILE OFF"
			"SDLMIXER_VORBIS_TREMOR OFF"
	)
endif()

CPMAddPackage(
	NAME freetype
	URL https://download.savannah.gnu.org/releases/freetype/freetype-2.14.1.tar.xz
	URL_HASH SHA256=32427e8c471ac095853212a37aef816c60b42052d4d9e48230bab3bdf2936ccc
	OPTIONS
		"FT_DISABLE_ZLIB ON"
		"FT_DISABLE_BZIP2 ON"
		"FT_DISABLE_PNG ON"
		"FT_DISABLE_HARFBUZZ ON"
		"FT_DISABLE_BROTLI ON"
)
if (EMSCRIPTEN)
	target_compile_options(freetype PRIVATE -sUSE_LIBPNG=1)
	target_link_options(freetype PRIVATE -sUSE_LIBPNG=1)
endif()

if (imgui_ADDED)
	set(IMGUI_SOURCES
		${imgui_SOURCE_DIR}/imgui.cpp
		${imgui_SOURCE_DIR}/imgui_demo.cpp
		${imgui_SOURCE_DIR}/imgui_draw.cpp
		${imgui_SOURCE_DIR}/imgui_tables.cpp
		${imgui_SOURCE_DIR}/imgui_widgets.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
	)
	add_library(imgui STATIC ${IMGUI_SOURCES})
	target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR};${imgui_SOURCE_DIR}/backends/")
	# target_link_libraries(imgui PUBLIC glew)
	target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW )

	if (EMSCRIPTEN)
		target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_ES3)
		target_compile_options(imgui PRIVATE -sUSE_SDL=3)
		target_link_options(imgui PRIVATE -sUSE_SDL=3)
	else()
		target_link_libraries(imgui PRIVATE SDL3::SDL3)
	endif()
endif()

if(implot_ADDED)

	set(IMPLOT_SOURCES
		${implot_SOURCE_DIR}/implot.cpp
		${implot_SOURCE_DIR}/implot_demo.cpp
		${implot_SOURCE_DIR}/implot_items.cpp
	)
	add_library(implot STATIC ${IMPLOT_SOURCES})
	target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
	target_include_directories(implot PUBLIC ${imgui_SOURCE_DIR})
	target_link_libraries(implot PUBLIC imgui)

	if (EMSCRIPTEN)
		target_compile_definitions(implot PRIVATE IMGUI_IMPL_OPENGL_ES3)
		target_compile_options(implot PRIVATE -sUSE_SDL=3)
		target_link_options(implot PRIVATE -sUSE_SDL=3)
	else()
		target_link_libraries(implot PRIVATE SDL3::SDL3)
	endif()
endif()

