
cmake_minimum_required (VERSION 3.8)


file(GLOB_RECURSE FF_RENDER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB_RECURSE FF_RENDER_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*)

add_library(ff_render STATIC
	${FF_RENDER_SOURCES}
	${FF_RENDER_HEADERS}
)
target_include_directories(ff_render PUBLIC "include/fastfall")
target_include_directories(ff_render PUBLIC "${glew_SOURCE_DIR}/include")
target_include_directories(ff_render PUBLIC "${sdl_SOURCE_DIR}/include")
target_include_directories(ff_render PUBLIC "${sdl_image_SOURCE_DIR}")
target_include_directories(ff_render PUBLIC "${PROJECT_SOURCE_DIR}/external/imgui")
target_include_directories(ff_render PUBLIC "${glm_SOURCE_DIR}")

target_compile_definitions(ff_render PUBLIC DEBUG)

target_link_libraries(ff_render PRIVATE 
	glm
	SDL2
	SDL2main
	SDL2image
	glew
	imgui
)