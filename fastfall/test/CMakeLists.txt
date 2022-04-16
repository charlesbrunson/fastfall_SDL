cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/google_test
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


macro(create_ff_test TESTNAME)

    add_executable(${TESTNAME} ${ARGN})
	target_link_libraries(${TESTNAME} gtest gtest_main fastfall)
		
	gtest_discover_tests(${TESTNAME}
		WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
	)
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)

endmacro()

enable_testing()

include(GoogleTest)

create_ff_test(ff_test_utils 
	utils/grid-vector.cpp
)
create_ff_test(ff_test_phys 
	phys/collision.cpp
	phys/surfacetracker.cpp

	phys/TestPhysRenderer.cpp
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/phys_render_out)

# add more tests...



