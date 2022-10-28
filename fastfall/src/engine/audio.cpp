#include "fastfall/engine/audio.hpp"

#include <cassert>

namespace ff {

SoLoud::Soloud *audio_engine = nullptr;

bool audio_init() {
    if (audio_engine == nullptr) {
        audio_engine = new SoLoud::Soloud{};
        audio_engine->init(
            SoLoud::Soloud::CLIP_ROUNDOFF,
            SoLoud::Soloud::SDL2,
            SoLoud::Soloud::AUTO,
            SoLoud::Soloud::AUTO,
            2
        );
    }
    return audio_is_init();
}

bool audio_quit() {
    if (audio_engine != nullptr) {
        audio_engine->deinit();
        delete audio_engine;
    }
    return !audio_is_init();
}

bool audio_is_init() {
    return audio_engine != nullptr;
}

SoLoud::Soloud& audio() {
    assert(audio_is_init());
    return *audio_engine;
}

}