#include "fastfall/resource/asset/SoundAsset.hpp"

#include <fastfall/engine/audio.hpp>

namespace ff {

SoundAsset::SoundAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

SoundAsset::~SoundAsset()
{
    if (loaded && audio::is_init()) ma_sound_uninit(&sound);
}

bool SoundAsset::loadFromFile() {
    auto str = asset_path.generic_string();

    if (audio::is_init())
    {
        loaded = ma_sound_init_from_file(
            &audio::get_engine(),
            str.c_str(),
            0,
            &audio::get_game_sound_group(),
            nullptr,
            &sound) == MA_SUCCESS;
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
