#include "fastfall/resource/asset/MusicAsset.hpp"

#include <SDL3_mixer/SDL_mixer.h>

namespace ff {

MusicAsset::MusicAsset(const std::filesystem::path &t_asset_path)
    : Asset(t_asset_path)
{
}

MusicAsset::~MusicAsset()
{
    Mix_FreeMusic(reinterpret_cast<Mix_Music*>(music_ptr));
}

bool MusicAsset::loadFromFile() {
    auto str = asset_path.generic_string();
    music_ptr = reinterpret_cast<Impl*>(Mix_LoadMUS(str.c_str()));
    loaded = music_ptr != nullptr;
    return loaded;
}

bool MusicAsset::reloadFromFile() {
    return true;
}

std::vector<std::filesystem::path> MusicAsset::getDependencies() const {
    return { asset_path };
}

}
