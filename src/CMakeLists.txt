cmake_minimum_required (VERSION 3.22)

add_subdirectory(util)
add_subdirectory(gfx)
add_subdirectory(external)

target_sources(ffengine
    PRIVATE
    engine.cpp
)
