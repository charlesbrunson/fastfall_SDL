#include "fastfall/engine/audio.hpp"

#include "fastfall/util/log.hpp"

#include "SDL3/SDL.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#undef MINIAUDIO_IMPLEMENTATION

#include "magic_enum/magic_enum.hpp"

namespace ff::audio {

bool init_state = false;

constexpr ma_format audio_format  = ma_format_f32;
constexpr int audio_channel_count = 2;
constexpr int audio_sample_rate   = 48000;

ma_engine audio_engine;
ma_node_graph audio_node_graph;

ma_sound_group sound_group_game;
ma_sound_group sound_group_music;

SDL_AudioDeviceID sdl_device = 0;

bool init() {

    ma_engine_config engine_config = ma_engine_config_init();
    engine_config.noDevice = MA_TRUE;
    engine_config.channels = audio_channel_count;
    engine_config.sampleRate = audio_sample_rate;

    ma_result result = ma_engine_init(&engine_config, &audio_engine);
    if (result != MA_SUCCESS)
    {
        LOG_INFO("Failed to init miniaudio");
        return false;
    }

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        LOG_INFO("Failed to init SDL audio subsystem: {}", SDL_GetError());
        return false;
    }

    SDL_AudioSpec desired_spec = {
        .format     = SDL_AUDIO_F32,
        .channels   = (int)ma_engine_get_channels(&audio_engine),
        .freq       = (int)ma_engine_get_sample_rate(&audio_engine),
    };

    auto sdl_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    SDL_ResumeAudioDevice(sdl_device);

    result = ma_sound_group_init(&audio_engine, 0, nullptr, &sound_group_game);
    if (result != MA_SUCCESS)
    {
        LOG_INFO("Failed to init sound group");
        return false;
    }

    result = ma_sound_group_init(&audio_engine, 0, nullptr, &sound_group_music);
    if (result != MA_SUCCESS)
    {
        LOG_INFO("Failed to init sound group");
        return false;
    }

    init_state = true;
    return init_state;
}

void quit() {
    ma_sound_group_uninit(&sound_group_game);
    ma_sound_group_uninit(&sound_group_music);
    ma_engine_uninit(&audio_engine);

    SDL_CloseAudioDevice(sdl_device);
    sdl_device = 0;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    init_state = false;
}

bool is_init() { return init_state; }

void set_master_volume(float volume) {
    ma_engine_set_volume(&audio_engine, std::clamp(volume, 0.f, 1.f));
}

void set_game_volume(float volume) {
    ma_sound_group_set_volume(&sound_group_game, volume);
}

void set_music_volume(float volume) {
    ma_sound_group_set_volume(&sound_group_music, volume);
}

float get_master_volume() {
    return ma_engine_get_volume(&audio_engine);
}

float get_game_volume() {
    return ma_sound_group_get_volume(&sound_group_game);
}

float get_music_volume() {
    return ma_sound_group_get_volume(&sound_group_music);
}

ma_engine& get_engine()
{
    return audio_engine;
}

ma_sound_group& get_game_sound_group()
{
    return sound_group_game;
}

ma_sound_group& get_music_sound_group()
{
    return sound_group_music;
}

}