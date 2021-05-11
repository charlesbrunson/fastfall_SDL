
cmake_minimum_required (VERSION 3.8)

add_subdirectory ("external")

file(GLOB_RECURSE FF_RENDER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB_RECURSE FF_RENDER_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*)

add_library(ff_render STATIC
	${FF_RENDER_SOURCES}
	${FF_RENDER_HEADERS}
)

target_include_directories(ff_render PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/fastfall")
target_include_directories(ff_render PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sdl/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/sdl_image
	${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/external/glm
)

target_compile_definitions(ff_render PRIVATE DEBUG)

target_link_libraries(ff_render PRIVATE 
	zlibstatic
	glm 
	SDL2-static 
	SDL2main 
	SDL2image 
	glew_s 
	imgui 
)