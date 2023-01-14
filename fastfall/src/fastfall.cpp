#include "fastfall/fastfall.hpp"

#include "fastfall/render.hpp"
#include "fastfall/engine/audio.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/resource/Resources.hpp"
#include "fastfall/resource/ResourceWatcher.hpp"
#include "fastfall/engine/imgui/ImGuiFrame.hpp"
#include "fastfall/engine/InputConfig.hpp"

namespace ff {

bool Init() {
    if (!render_init()) {
        LOG_ERR_("Could not initialize render subsystem");
        return false;
    }

    if (!audio_init()) {
        LOG_ERR_("Could not initialize audio subsystem");
        return false;
    }

    if (InputConfig::configExists()) {
        InputConfig::readConfigFile();
    } else {
        LOG_INFO("No input configuration file, creating");
        InputConfig::writeConfigFile();
    }

    LOG_INFO("fastfall initialization complete");
    return true;
}

bool Load_Resources() {
    if (!render_glew_is_init()) {
        LOG_ERR_("Cannot load resources without an OpenGL context, a Window must be created first");
        return false;
    }

    bool result = Resources::loadAll()
               && Resources::compileShaders();
    if (!result) {
        LOG_ERR_("Could not load assets");
    } else {
#if not defined(__EMSCRIPTEN__)
        Resources::addLoadedToWatcher();
        ResourceWatcher::start_watch_thread();
#endif
    }

    return result;
}

void Quit() {
#if not defined(__EMSCRIPTEN__)

    bool kill_watch = ResourceWatcher::is_watch_running();

    if (kill_watch)
        ResourceWatcher::stop_watch_thread();

    Resources::unloadAll();
    ImGuiFrame::getInstance().clear();

    audio_quit();
    render_quit();

    if (kill_watch)
        ResourceWatcher::join_watch_thread();
#endif
}

}
