#include "fastfall/game/systems/AudioSystem.hpp"

#include "fastfall/engine/audio.hpp"

#include "fastfall/engine/Engine.hpp"

//TODO: replace SoLoud

namespace ff {
namespace audio {
    void Volume::operator()(SoundHandle t_hdl) {
        // engine().setVolume(t_hdl.id, loudness);
    }

    void Pan::operator()(SoundHandle t_hdl) {
        // engine().setPan(t_hdl.id, pan);
    }
}


AudioSystem::AudioSystem(audio::game_bus_t* t_dest)
    : dest(t_dest)
{
}

void AudioSystem::set_pause_all(bool t_paused) {
    for (auto& hdl : active_sounds) {
        // audio::engine().setPause(hdl.id, t_paused);
    }
}

void AudioSystem::set_pause(SoundHandle t_hdl, bool t_paused) {
    // audio::engine().setPause(t_hdl.id, t_paused);
}

void AudioSystem::stop(SoundHandle t_hdl) {
    // audio::engine().stop(t_hdl.id);
}

void AudioSystem::update(secs deltaTime) {
    upTime += deltaTime;
}

SoundAsset* AudioSystem::get_asset_impl(std::string_view sound_asset_name) {
    return Resources::get<SoundAsset>(sound_asset_name);
}

SoundHandle AudioSystem::play_impl(SoundAsset &sound_asset) {
    if (dest) {
        SoundHandle hdl = { /* dest->game.playClocked(upTime, sound_asset.wav()) */ };
        if (!hdl.id) {
            LOG_ERR_("Couldn't play {}", sound_asset.get_name());
        }
        return hdl;
    }
    return { 0 };
}

}