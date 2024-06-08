cmake_minimum_required (VERSION 3.22)

macro(create_ff_test TESTNAME)

    add_executable(${TESTNAME} ${ARGN})
	target_link_libraries(${TESTNAME} PRIVATE GTest::gtest GTest::gtest_main ffengine)
	# target_include_directories(${TESTNAME} PRIVATE "google_test/googletest/include")

	gtest_discover_tests(${TESTNAME}
		WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
	)
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

enable_testing()
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

create_ff_test(ff_test_util
	util/test_grid_vector.cpp
	util/test_copyable_unique_ptr.cpp
)

create_ff_test(ff_test_core
	core/test_clock.cpp
)
# add more tests...
