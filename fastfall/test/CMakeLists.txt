cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/google_test
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

macro(create_ff_test test_cpp)

	string(REPLACE "/" "_" test_name ${test_cpp})
	string(REPLACE ".cpp" "" test_name ${test_name})
	
	message(STATUS "AAAAAAAAAAA ${test_name}  -  ${test_cpp}")

	add_executable(${test_name} ${test_cpp})
	target_link_libraries(${test_name} gtest_main fastfall)
	target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/google_test/google_test/include)
	gtest_discover_tests(${test_name})
	message(STATUS "built test ${test_name}")
	
	if (DEFINED FF_TESTS)
		list(APPEND FF_TESTS ${test_name})
	else()
		set(FF_TESTS ${test_name})
	endif()
endmacro()

create_ff_test("util/grid-vector.cpp")
create_ff_test("game/phys/collision.cpp")
# add more tests...
