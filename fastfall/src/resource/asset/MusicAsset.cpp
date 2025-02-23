#include "fastfall/resource/asset/MusicAsset.hpp"

#include <fastfall/engine/audio.hpp>

#include "miniaudio.h"

namespace ff {

MusicAsset::MusicAsset(const std::filesystem::path &t_asset_path)
    : Asset(t_asset_path)
{
}

MusicAsset::~MusicAsset()
{
    if (loaded) ma_sound_uninit(&music);
}

bool MusicAsset::loadFromFile() {
    auto str = asset_path.generic_string();

    loaded = ma_sound_init_from_file(
        &audio::get_engine(),
        str.c_str(),
        MA_SOUND_FLAG_STREAM,
        &audio::get_music_sound_group(),
        nullptr,
        &music) == MA_SUCCESS;

    return loaded;
}

bool MusicAsset::reloadFromFile() {
    return true;
}

std::vector<std::filesystem::path> MusicAsset::getDependencies() const {
    return { asset_path };
}

}
