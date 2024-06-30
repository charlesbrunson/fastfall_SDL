#include "ff/engine.hpp"

#include "ff/util/log.hpp"

#include "external/sdl.hpp"

#include "external/freetype.hpp"
#include "external/imgui.hpp"

namespace ff {

bool has_engine = false;

engine::engine() {
    if (has_engine)
        throw std::runtime_error("engine already running");

    info("Initializing ffengine");
    sdl_init();
    freetype_init();
    has_engine = true;
}

engine::~engine() {
    if (has_engine) {
        info("Shutting down ffengine");
        freetype_quit();
        sdl_quit();
        has_engine = false;
    }
}

/*
bool initialize() {
    info("Initializing ffengine");
    sdl_init();
    freetype_init();
    return true;
}

bool shutdown() {
    info("Shutting down ffengine");
    freetype_quit();
    sdl_quit();
    return true;
}
*/

}