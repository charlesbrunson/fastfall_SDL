# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.19)

project ("fastfall_test_project")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# build fastfall engine.
#add_subdirectory ("fastfall")

add_subdirectory ("fastfall")

add_executable(test_project	
	"main.cpp" 
	"EmptyState.hpp"
	"content/object/BasicPlatform.hpp" 
	"content/object/Player.cpp" 
	"content/object/Player.hpp" 
	"content/TestState.cpp" 
	"content/TestState.hpp" "content/object/BasicPlatform.cpp")

target_include_directories(test_project PRIVATE

	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/rapidxml
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/fmt/include
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/flatbuffers/include
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/glm
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/glew/include
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/sdl/include
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/sdl_image
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/fastfall/external/implot
)

target_link_libraries(test_project PRIVATE 
	fastfall
)

target_compile_definitions(test_project PRIVATE DEBUG)
