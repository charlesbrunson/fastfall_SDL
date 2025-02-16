#pragma once

// #include "soloud.h"

// TODO: replace SoLoud
namespace ff::audio {

struct game_bus_t {
    // SoLoud::Bus music;
    // SoLoud::Bus game;
};

enum class AudioBackends {
    SDL2, // = SoLoud::Soloud::BACKENDS::SDL2,
    Null, // = SoLoud::Soloud::BACKENDS::NULLDRIVER
};

bool init(AudioBackends backend = AudioBackends::SDL2);
bool quit();
bool is_init();

void set_master_volume(float volume);
void set_game_volume(float volume);
void set_music_volume(float volume);

float get_master_volume();
float get_game_volume();
float get_music_volume();

game_bus_t& primary_bus();
// SoLoud::Soloud& engine();

}