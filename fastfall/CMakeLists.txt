# CMakeList.txt : CMake project for fastfall_SDL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)



# build fastfall util library
add_subdirectory ("util")

# build fastfall render library
add_subdirectory ("render")

# build fastfall engine library


# build fastfall game library

add_executable (fastfall_test 
	"main.cpp"
)

target_compile_definitions(fastfall_test PUBLIC DEBUG)

target_link_libraries(fastfall_test PRIVATE 
	${FF_UTIL_LIBRARY}
	${FF_RENDER_LIBRARY}
)
target_include_directories(fastfall_test PRIVATE 
	${FF_UTIL_INCLUDE}
	${FF_RENDER_INCLUDE}
)
