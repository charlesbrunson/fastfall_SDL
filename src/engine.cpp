#include "ff/engine.hpp"

#include "ff/util/log.hpp"

#include "external/sdl.hpp"

#include "external/freetype.hpp"
#include "external/imgui.hpp"

namespace ff {

bool initialize() {
    info("Initializing ffengine");
    sdl_init();
    freetype_init();
    return true;
}

bool shutdown() {
    info("Shutting down ffengine");
    imgui_quit();
    freetype_quit();
    sdl_quit();
    return true;
}

}