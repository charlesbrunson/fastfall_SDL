#include "fastfall/resource/asset/SoundAsset.hpp"

namespace ff {

SoundAsset::SoundAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

bool SoundAsset::loadFromFile() {
    auto str = asset_path.generic_string();
    loaded = sound.load(str.data()) == SoLoud::SO_NO_ERROR;
    return loaded;
}

bool SoundAsset::reloadFromFile() {
    return loadFromFile();
}

std::vector<std::filesystem::path> SoundAsset::getDependencies() const {
    return {
        asset_path
    };
}

}