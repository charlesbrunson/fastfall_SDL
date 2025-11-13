cmake_minimum_required (VERSION 3.25)

add_subdirectory(util)
add_subdirectory(resource)
add_subdirectory(render)
add_subdirectory(engine)
add_subdirectory(game)

target_sources(fastfall PRIVATE
        fastfall.cpp
        user_types.cpp)