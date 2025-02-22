#pragma once
#include "SDL3/SDL_audio.h"

// TODO: replace SoLoud
namespace ff::audio {

bool init();
void quit();
bool is_init();

void set_master_volume(float volume);
void set_game_volume(float volume);
void set_music_volume(float volume);

float get_master_volume();
float get_game_volume();
float get_music_volume();

}