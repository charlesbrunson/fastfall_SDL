
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory ("external")

file(GLOB_RECURSE FF_RENDER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB_RECURSE FF_RENDER_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*)

add_library(ff_render STATIC
	${FF_RENDER_SOURCES}
	${FF_RENDER_HEADERS}
)

set(FF_RENDER_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "fastfall render include path")
set(FF_RENDER_LIBRARY ff_render							  CACHE STRING "fastfall render library name")

target_include_directories(ff_render PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_include_directories(ff_render PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sdl/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sdl_image
	${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
	${FF_UTIL_INCLUDE}
)

target_compile_definitions(ff_render PRIVATE DEBUG)

target_link_libraries(ff_render PRIVATE 
	${FF_UTIL_LIBRARY}
	zlibstatic
	SDL2-static 
	SDL2main 
	SDL2image 
	glew_s 
	imgui 
)