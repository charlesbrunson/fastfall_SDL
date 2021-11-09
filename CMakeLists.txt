# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.16)

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
	"content/object/Player.cpp" 
	"content/object/Player.hpp" 
	"content/object/PlayerCommon.hpp" 
	"content/object/PlayerCommon.cpp" 
	"content/object/plr_states/PlayerAir.hpp" 
	"content/object/plr_states/PlayerAir.cpp" 
	"content/object/plr_states/PlayerGround.hpp"
	"content/object/plr_states/PlayerGround.cpp"
	"content/object/plr_states/PlayerDash.hpp"
	"content/object/plr_states/PlayerDash.cpp"
	"content/object/BasicPlatform.hpp" 
	"content/object/BasicPlatform.cpp" 
	"content/TestState.cpp" 
	"content/TestState.hpp" 
	"content/tilelogic/AnimLogic.hpp"
	"content/tilelogic/AnimLogic.cpp"
	"content/types.hpp"
	"content/types.cpp"
	"content/camera/SimpleCamTarget.hpp"
	"content/camera/SimpleCamTarget.cpp" 
	
	)

target_link_libraries(test_project PRIVATE 
	fastfall
)

target_compile_definitions(test_project PUBLIC DEBUG)

if(EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX .html)
	set(USE_FLAGS "-s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"bmp\",\"png\"]' -s FULL_ES3=1 -s WASM=1 -s OFFSCREEN_FRAMEBUFFER=1 -g --preload-file ../data@/data --preload-file ../gamecontrollerdb.txt@/gamecontrollerdb.txt -s ALLOW_MEMORY_GROWTH=1")
	set_target_properties(test_project PROPERTIES LINK_FLAGS "${USE_FLAGS} --emrun")
	set_target_properties(test_project PROPERTIES COMPILE_FLAGS ${USE_FLAGS})
endif()
