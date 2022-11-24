# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.16)

project ("fastfall_test_project")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "C Compiler executable:   ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler executable: ${CMAKE_CXX_COMPILER}")
message(STATUS "Linker executable:       ${CMAKE_LINKER}")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

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
	"content/object/JetPlatform.hpp"
	"content/object/JetPlatform.cpp"
	"content/TestState.cpp"
	"content/TestState.hpp" 
	"content/tilelogic/AnimLogic.hpp"
	"content/tilelogic/AnimLogic.cpp"
	"content/types.hpp"
	"content/types.cpp"
	"content/camera/SimpleCamTarget.hpp"
	"content/camera/SimpleCamTarget.cpp" 
	"content/object/SimpleEffect.hpp" 
	"content/object/SimpleEffect.cpp")

target_link_libraries(test_project PRIVATE 
	fastfall
)

if(EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX .html)
	
	target_link_options(test_project PRIVATE
		--emrun
	)
	
	#if (${CMAKE_BUILD_TYPE} MATCHES "Release")
	#	target_link_options(test_project PRIVATE
	#		--shell-file "${CMAKE_CURRENT_SOURCE_DIR}/html/min-shell.html"
	#	)
	#endif()

	# create test_project.data file
	target_link_options(test_project PRIVATE 
		"SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/data@/data" 
		"SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/gamecontrollerdb.txt@/gamecontrollerdb.txt"
	)

	# rename test_project.html to index.html
	add_custom_command(
		TARGET test_project 
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rename 
		"$<TARGET_FILE_DIR:test_project>/test_project${CMAKE_EXECUTABLE_SUFFIX}"
		"$<TARGET_FILE_DIR:test_project>/index${CMAKE_EXECUTABLE_SUFFIX}")

	# zip it
	add_custom_command(
		TARGET test_project 
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "test_project_${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_SYSTEM_PROCESSOR}.zip" 
			--format=zip
			"$<TARGET_FILE_DIR:test_project>/index${CMAKE_EXECUTABLE_SUFFIX}"
			"$<TARGET_FILE_DIR:test_project>/test_project.js"
			"$<TARGET_FILE_DIR:test_project>/test_project.wasm"
			"$<TARGET_FILE_DIR:test_project>/test_project.data")

else()
	if (${CMAKE_BUILD_TYPE} MATCHES "Release")

		# copy data directory to bin folder
		add_custom_command(
			TARGET test_project 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
        		${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:test_project>/data/)
			
		# copy gamecontrollerdb.txt directory to bin folder
		add_custom_command(
			TARGET test_project 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
        		${CMAKE_SOURCE_DIR}/gamecontrollerdb.txt $<TARGET_FILE_DIR:test_project>)
			
		# zip it
		add_custom_command(
			TARGET test_project 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "test_project_${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_SYSTEM_PROCESSOR}.zip" 
				--format=zip
				"$<TARGET_FILE_DIR:test_project>/test_project${CMAKE_EXECUTABLE_SUFFIX}"
				"$<TARGET_FILE_DIR:test_project>/data"
				"$<TARGET_FILE_DIR:test_project>/gamecontrollerdb.txt")
	endif()
endif()

