cmake_minimum_required (VERSION 3.22)

target_sources(ffengine
    PRIVATE
        base64.cpp
        log.cpp
        file_slurp.cpp
)