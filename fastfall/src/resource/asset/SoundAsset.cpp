#include "fastfall/resource/asset/SoundAsset.hpp"

#include "fastfall/engine/audio.hpp"
#include <SDL3_mixer/SDL_mixer.h>

#include "fastfall/util/log.hpp"

namespace ff {

SoundAsset::SoundAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

SoundAsset::~SoundAsset() {
    if (audio::is_init() && audio_impl) {
        MIX_DestroyAudio(audio_impl);
    }
}

bool SoundAsset::loadFromFile() {
    auto str = asset_path.generic_string();


    loaded = false;
    if (audio::is_init())
    {
        if (audio_impl) {
            MIX_DestroyAudio(audio_impl);
            audio_impl = nullptr;
        }

        audio_impl = MIX_LoadAudio(audio::get_mixer(), str.c_str(), true);
        loaded = audio_impl != nullptr;
        if (!audio_impl) {
            LOG_WARN("Fail to load {}: {}", str, SDL_GetError());
        }
    }

    return loaded;
}

bool SoundAsset::reloadFromFile() {
    return loadFromFile();
}

std::vector<std::filesystem::path> SoundAsset::getDependencies() const {
    return { asset_path };
}

}
