# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.19)

project ("fastfall_test_project")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "C Compiler executable:   ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler executable: ${CMAKE_CXX_COMPILER}")
message(STATUS "Linker executable:       ${CMAKE_LINKER}")

add_subdirectory ("fastfall")

add_executable(test_project	
	"main.cpp" 
	"EmptyState.hpp"
	"content/object/BasicPlatform.hpp" 
	"content/object/Player.cpp" 
	"content/object/Player.hpp" 
	"content/TestState.cpp" 
	"content/TestState.hpp" 
	"content/object/BasicPlatform.cpp"
)

target_link_libraries(test_project PRIVATE 
	fastfall
)

target_compile_definitions(test_project PUBLIC DEBUG)
