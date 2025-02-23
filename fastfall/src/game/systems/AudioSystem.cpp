#include "fastfall/game/systems/AudioSystem.hpp"

#include "fastfall/engine/audio.hpp"

#include "fastfall/engine/Engine.hpp"

//TODO: replace SoLoud

namespace ff {
namespace audio {
    void Volume::operator()(SoundHandle t_hdl) {
        ma_sound_set_volume(&t_hdl.sound, loudness);
    }

    void Pan::operator()(SoundHandle t_hdl) {
        ma_sound_set_pan(&t_hdl.sound, pan);
    }
}

void AudioSystem::set_pause_all(bool t_paused) {
    for (auto& hdl : active_sounds) {
        ma_sound_stop(&hdl.sound);
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
        auto to_erase = !ma_sound_is_playing(&hdl.sound);
        if (to_erase) ma_sound_uninit(&hdl.sound);
        return to_erase;
    });
    active_sounds.erase(it, active_sounds.end());
}

SoundAsset* AudioSystem::get_asset_impl(std::string_view sound_asset_name) {
    return Resources::get<SoundAsset>(sound_asset_name);
}

SoundHandle AudioSystem::play_impl(SoundAsset& sound_asset) {
    if (audio::is_init()) {
        auto next_id = id_counter++;
        if (next_id == 0)
        {
            next_id = id_counter++;
        }

        SoundHandle hdl = { .id = next_id };

        auto result = ma_sound_init_from_file(
            &audio::get_engine(),
            sound_asset.get_path().c_str(),
            MA_SOUND_FLAG_DECODE,
            &audio::get_game_sound_group(),
            nullptr,
            &hdl.sound) == MA_SUCCESS;

        if (!result) {
            LOG_ERR_("Couldn't play {}", sound_asset.get_name());
            hdl.id = 0;
        }
        else
        {
            ma_sound_start(&hdl.sound);
        }

        return hdl;
    }
    return {};
}

}