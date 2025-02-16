#include "fastfall/engine/audio.hpp"
#include "fastfall/util/log.hpp"

#include "SDL.h"

namespace ff::audio {

// TODO: replace SoLoud
// SoLoud::Soloud audio_engine;

game_bus_t primary_mix;
float game_v  = 1.f;
float music_v = 1.f;
float master_v = 0.5f;

bool init_state = false;

bool init(AudioBackends backend) {
    /*
    auto r = audio_engine.init(
        SoLoud::Soloud::CLIP_ROUNDOFF,
        static_cast<SoLoud::Soloud::BACKENDS>(backend),
        SoLoud::Soloud::AUTO,
        SoLoud::Soloud::AUTO
    );

    if (r != SoLoud::SOLOUD_ERRORS::SO_NO_ERROR) {
        LOG_ERR_("Failed to initialize audio: {}", r);
        std::string err = SDL_GetError();
        LOG_ERR_("SDL: {}", err);
        init_state = false;
    } else {
        LOG_INFO("    SoLoud {}", SOLOUD_VERSION);
        audio_engine.setMaxActiveVoiceCount(64);
        audio_engine.setGlobalVolume(master_v);
        primary_mix.game.setVolume(game_v);
        primary_mix.music.setVolume(music_v);
        audio_engine.play(primary_mix.game);
        audio_engine.play(primary_mix.music);
        init_state = true;
    }
    return init_state;
    */
    init_state = true;
    return init_state;
}

bool quit() {
    // audio_engine.deinit();
    init_state = false;
    return !init_state;
}

bool is_init() { return init_state; }

void set_master_volume(float volume) {
    master_v = volume;
    // audio_engine.setGlobalVolume(master_v);
}

void set_game_volume(float volume) {
    game_v = volume;
    // primary_mix.game.setVolume(game_v);
}

void set_music_volume(float volume) {
    music_v = volume;
    // primary_mix.game.setVolume(music_v);
}

float get_master_volume() { return master_v; }
float get_game_volume()   { return game_v;   }
float get_music_volume()  { return music_v;  }

game_bus_t& primary_bus() { return primary_mix; }
// SoLoud::Soloud& engine() { return audio_engine; }

}