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
	target_include_directories(${TESTNAME} PRIVATE "google_test/googletest/include")
		
		
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
	utils/copyable-unique.cpp
	utils/dmessage.cpp
)


create_ff_test(ff_test_phys 
	phys/collision.cpp
	phys/surfacetracker.cpp
	phys/colliderghost.cpp

	phys/TestPhysRenderer.cpp
)

create_ff_test(ff_test_pathfollow
	phys/path_follow.cpp
)

create_ff_test(ff_test_engine
	engine/statehandler.cpp
	engine/input.cpp
	engine/inputstate.cpp
)

create_ff_test(ff_test_particle
	particle/particle.cpp
	particle/ParticleRenderer.cpp
)

create_ff_test(ff_test_audio
	audio/audio_system.cpp
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/phys_render_out)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/particle_render_out)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# add more tests...



