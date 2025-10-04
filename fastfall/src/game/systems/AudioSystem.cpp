#include "fastfall/game/systems/AudioSystem.hpp"

#include "fastfall/engine/audio.hpp"

#include "fastfall/engine/Engine.hpp"

namespace ff {

void AudioSystem::update(secs deltaTime) {
    upTime += deltaTime;
}

SoundAsset* AudioSystem::get_asset_impl(std::string_view sound_asset_name) {
    return Resources::get<SoundAsset>(sound_asset_name);
}

}