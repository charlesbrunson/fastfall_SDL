#include "fastfall/resource/asset/SoundAsset.hpp"

#include <SDL3_mixer/SDL_mixer.h>

namespace ff {

SoundAsset::SoundAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

SoundAsset::~SoundAsset()
{
    Mix_FreeChunk(reinterpret_cast<Mix_Chunk*>(sound_ptr));
}

bool SoundAsset::loadFromFile() {
    auto str = asset_path.generic_string();
    sound_ptr = reinterpret_cast<Impl*>(Mix_LoadWAV(str.c_str()));
    loaded = sound_ptr != nullptr;
    loaded = true;
    return loaded;
}

bool SoundAsset::reloadFromFile() {
    return loadFromFile();
}

std::vector<std::filesystem::path> SoundAsset::getDependencies() const {
    return { asset_path };
}

}
