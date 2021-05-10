# CMakeList.txt : CMake project for fastfall_SDL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


add_subdirectory ("render")

add_executable (fastfall_SDL 
	"main.cpp"
	 )
	 
target_link_libraries(fastfall_SDL PRIVATE 
	ff_render
)
target_include_directories(fastfall_SDL PRIVATE "render/include")