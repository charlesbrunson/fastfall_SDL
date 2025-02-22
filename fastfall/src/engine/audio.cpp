#include "fastfall/engine/audio.hpp"

#include "fastfall/util/log.hpp"

#include "SDL3/SDL.h"
#include "SDL3_mixer/SDL_mixer.h"

#include "magic_enum/magic_enum.hpp"

namespace ff::audio {

bool init_state = false;

bool init() {
    SDL_AudioSpec spec;
    spec.channels = MIX_DEFAULT_CHANNELS;
    spec.freq     = MIX_DEFAULT_FREQUENCY;
    spec.format   = MIX_DEFAULT_FORMAT;

    if (Mix_OpenAudio(0, &spec))
    {
        Mix_QuerySpec(&spec.freq, &spec.format, &spec.channels);
        LOG_INFO("Opened audio at {} Hz {} bit{} {}",
            spec.freq,
            spec.format&0xFF,
            (SDL_AUDIO_ISFLOAT(spec.format) ? " (float)" : ""),
            (spec.channels > 2 ? "surround" : (spec.channels > 1 ? "stereo" : "mono")));
    }
    else
    {
        LOG_INFO("Failed to open audio");
        return false;
    }

    init_state = true;
    return init_state;
}

void quit() {
    Mix_CloseAudio();
    init_state = false;
}

bool is_init() { return init_state; }

void set_master_volume(float volume) {
    auto vol_converted = static_cast<int>(std::clamp(volume, 0.f, 1.f) * MIX_MAX_VOLUME);
    Mix_MasterVolume(vol_converted);
}

void set_game_volume(float volume) {
    auto vol_converted = static_cast<int>(std::clamp(volume, 0.f, 1.f) * MIX_MAX_VOLUME);
    Mix_Volume(-1, vol_converted);
}

void set_music_volume(float volume) {
    auto vol_converted = static_cast<int>(std::clamp(volume, 0.f, 1.f) * MIX_MAX_VOLUME);
    Mix_VolumeMusic(vol_converted);
}

float get_master_volume() {
    return static_cast<float>(Mix_MasterVolume(-1)) / MIX_MAX_VOLUME;
}

float get_game_volume() {
    return static_cast<float>(Mix_Volume(-1, -1)) / MIX_MAX_VOLUME;
}

float get_music_volume() {
    return static_cast<float>(Mix_VolumeMusic(-1)) / MIX_MAX_VOLUME;
}

}