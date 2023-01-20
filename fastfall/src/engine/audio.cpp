#include "fastfall/engine/audio.hpp"
#include "fastfall/util/log.hpp"

#include "SDL.h"

#include <cassert>

namespace ff {

SoLoud::Soloud audio_engine;
bool is_init = false;

bool audio_init() {
    auto r = audio_engine.init(
        SoLoud::Soloud::CLIP_ROUNDOFF,
        SoLoud::Soloud::BACKENDS::SDL2,
        SoLoud::Soloud::AUTO,
        SoLoud::Soloud::AUTO,
        2
    );

    if (r != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR) {
        LOG_ERR_("Failed to initialize audio: {}", r);
        std::string err = SDL_GetError();
        LOG_ERR_("SDL: {}", err);
        is_init = false;
    }
    else {
        audio_engine.setMaxActiveVoiceCount(64);
        is_init = true;
    }
    return is_init;
}

bool audio_quit() {
    audio_engine.deinit();
    is_init = false;
    return !is_init;
}

bool audio_is_init() {
    return is_init;
}

SoLoud::Soloud& audio() {
    return audio_engine;
}

}