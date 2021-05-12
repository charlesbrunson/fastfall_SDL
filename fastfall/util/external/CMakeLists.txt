
cmake_minimum_required (VERSION 3.19)

include(FetchContent)

# glm
FetchContent_Declare(
	glm
	URL			https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

#fmt
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        7.1.3
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/fmt
)

#flatbuffers
set(FLATBUFFERS_BUILD_TESTS OFF)
FetchContent_Declare(
	flatbuffers
	GIT_REPOSITORY https://github.com/google/flatbuffers.git
	GIT_TAG        v1.12.0
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/flatbuffers
)

#rapidxml
FetchContent_Declare(
	rapidxml
	GIT_REPOSITORY https://github.com/discord/rapidxml.git
	GIT_TAG        2ae4b2888165a393dfb6382168825fddf00c27b9
    SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/rapidxml
)

FetchContent_MakeAvailable(
	glm
	fmt
	flatbuffers
	rapidxml
)


