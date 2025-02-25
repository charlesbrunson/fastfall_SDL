#include "fastfall/game/systems/AudioSystem.hpp"

#include "fastfall/engine/audio.hpp"

#include "fastfall/engine/Engine.hpp"

namespace ff {
namespace audio {
    void Volume::operator()(ma_sound* sound) {
        ma_sound_set_volume(sound, loudness);
    }

    void Pan::operator()(ma_sound* sound) {
        ma_sound_set_pan(sound, pan);
    }
}

void AudioSystem::set_pause_all(bool t_paused) {
    for (auto hdl : active_sounds) {
        ma_sound_stop(hdl.sound);
    }
}

void AudioSystem::set_pause(ma_sound* sound, bool t_paused) {
    if (t_paused)
    {
        ma_sound_group_stop(&audio::get_game_sound_group());
    }
    else
    {
        ma_sound_group_start(&audio::get_game_sound_group());
    }
}

void AudioSystem::stop(ma_sound* t_hdl) {
    ma_sound_group_stop(&audio::get_game_sound_group());
}

void AudioSystem::update(secs deltaTime) {
    upTime += deltaTime;

    erase_if(active_sounds, [](const SoundHandle& hdl) {
        if (hdl.sound != nullptr && !ma_sound_is_playing(hdl.sound)) {
            ma_sound_uninit(hdl.sound);
            ma_resource_manager_data_source_uninit(hdl.data_source);

            delete hdl.sound;
            delete hdl.data_source;

            return true;
        }
        return false;
    });
}

SoundAsset* AudioSystem::get_asset_impl(std::string_view sound_asset_name) {
    return Resources::get<SoundAsset>(sound_asset_name);
}

SoundHandle AudioSystem::play_impl(SoundAsset& sound_asset) {
    if (!audio::is_init()) {
        return {};
    }

    bool result;
    SoundHandle hdl;
    hdl.data_source = new ma_resource_manager_data_source();
    result = ma_resource_manager_data_source_init_copy(
        audio::get_engine().pResourceManager,
        sound_asset.get_data_source(),
        hdl.data_source) == MA_SUCCESS;

    if (!result) {
        delete hdl.data_source;
        LOG_INFO("Failed to load sound: {}, {}", sound_asset.get_name());
        return {};
    }

    hdl.sound = new ma_sound();
    result = ma_sound_init_from_data_source(
        &audio::get_engine(),
        hdl.data_source,
        0,
        &audio::get_game_sound_group(),
        hdl.sound) == MA_SUCCESS;

    if (!result) {
        delete hdl.sound;
        ma_resource_manager_data_source_uninit(hdl.data_source);
        delete hdl.data_source;
        LOG_INFO("Failed to play sound: {}", sound_asset.get_name());
        return {};
    }

    return hdl;
}

}