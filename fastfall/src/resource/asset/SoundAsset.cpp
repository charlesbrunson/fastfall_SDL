#include "fastfall/resource/asset/SoundAsset.hpp"

namespace ff {

SoundAsset::SoundAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

bool SoundAsset::loadFromFile() {
    loaded = sound.load(asset_path.c_str()) == SoLoud::SO_NO_ERROR;
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