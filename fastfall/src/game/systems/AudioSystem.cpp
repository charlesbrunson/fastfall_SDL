#include "fastfall/game/systems/AudioSystem.hpp"

#include "fastfall/engine/audio.hpp"

#include "fastfall/engine/Engine.hpp"

//TODO: replace SoLoud

namespace ff {
namespace audio {
    void Volume::operator()(SoundHandle t_hdl) {
        ma_sound_set_volume(t_hdl.sound, loudness);
    }

    void Pan::operator()(SoundHandle t_hdl) {
        ma_sound_set_pan(t_hdl.sound, pan);
    }
}

void AudioSystem::set_pause_all(bool t_paused) {
    for (auto& hdl : active_sounds) {
        ma_sound_stop(hdl.sound);
    }
}

void AudioSystem::set_pause(SoundHandle t_hdl, bool t_paused) {
    if (t_paused)
    {
        ma_sound_group_stop(&audio::get_game_sound_group());
    }
    else
    {
        ma_sound_group_start(&audio::get_game_sound_group());
    }
}

void AudioSystem::stop(SoundHandle t_hdl) {
    ma_sound_group_stop(&audio::get_game_sound_group());
}

void AudioSystem::update(secs deltaTime) {
    upTime += deltaTime;

    auto it = std::remove_if(active_sounds.begin(), active_sounds.end(), [](SoundHandle& hdl)
    {
        auto to_erase = hdl.sound == nullptr || !ma_sound_is_playing(hdl.sound);
        if (to_erase && hdl.sound != nullptr)
        {
            ma_sound_uninit(hdl.sound);
            delete hdl.sound;
        }
        return to_erase;
    });
    active_sounds.erase(it, active_sounds.end());
}

SoundAsset* AudioSystem::get_asset_impl(std::string_view sound_asset_name) {
    return Resources::get<SoundAsset>(sound_asset_name);
}

SoundHandle AudioSystem::play_impl(SoundAsset& sound_asset) {
    if (audio::is_init()) {

        SoundHandle hdl = { /* .sound = new ma_sound */ };

        /*
        auto result = ma_sound_init_copy(
            &audio::get_engine(),
            &sound_asset.get_sound(),
            0,
            &audio::get_game_sound_group(),
            hdl.sound) == MA_SUCCESS;

        if (result != MA_SUCCESS)
        {
            delete hdl.sound;
            hdl.sound = nullptr;
            LOG_INFO("Failed to play sound: {}", sound_asset.get_name());
        }
        */

        return hdl;
    }
    return {};
}

}