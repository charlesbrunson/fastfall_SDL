#include "fastfall/resource/asset/MusicAsset.hpp"

#include "fastfall/engine/audio.hpp"

#include <SDL3/SDL_iostream.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "fastfall/util/log.hpp"

namespace ff {

MusicAsset::MusicAsset(const std::filesystem::path &t_asset_path)
    : Asset(t_asset_path)
{
}

MusicAsset::~MusicAsset()
{
    if (audio::is_init() && music_impl) {
        SDL_CloseIO(music_impl);
    }
}


bool MusicAsset::loadFromFile() {
    auto str = asset_path.generic_string();

    loaded = false;
    if (audio::is_init())
    {
        if (music_impl) {
            SDL_CloseIO(music_impl);
            music_impl = nullptr;
        }

        music_impl = SDL_IOFromFile(str.c_str(), "rb");

        loaded = music_impl != nullptr;
        if (!loaded) {
            LOG_WARN("Failed to load {}: {}", str, SDL_GetError());
        }
    }

    return loaded;
}

bool MusicAsset::reloadFromFile() {
    return true;
}

std::vector<std::filesystem::path> MusicAsset::getDependencies() const {
    return { asset_path };
}

}
