#include "fastfall/resource/asset/SoundAsset.hpp"

#include <fastfall/engine/audio.hpp>

namespace ff {

void ma_sound_deleter::operator()(ma_sound* sound)
{
    if (sound && audio::is_init())
    {
        ma_sound_uninit(sound);
    }
    delete sound;
}

SoundAsset::SoundAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

bool SoundAsset::loadFromFile() {
    auto str = asset_path.generic_string();

    sound.reset();

    loaded = false;
    if (audio::is_init())
    {
        sound.reset(new ma_sound);
        loaded = ma_sound_init_from_file(
            &audio::get_engine(),
            str.c_str(),
            0,
            &audio::get_game_sound_group(),
            nullptr,
            sound.get()) == MA_SUCCESS;
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
