#pragma once
#include "miniaudio.h"

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

ma_engine& get_engine();
ma_sound_group& get_game_sound_group();
ma_sound_group& get_music_sound_group();

}