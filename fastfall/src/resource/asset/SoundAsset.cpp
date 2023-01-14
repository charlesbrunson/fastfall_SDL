#include "fastfall/resource/asset/SoundAsset.hpp"

namespace ff {

SoundAsset::SoundAsset(const std::string& filename)
    : Asset(filename)
{
}

bool SoundAsset::loadFromFile(const std::string& relpath) {
    assetFilePath = relpath;
    loaded = sound.load((assetFilePath + assetName).c_str()) == SoLoud::SO_NO_ERROR;
    return loaded;
}

bool SoundAsset::reloadFromFile() {
    return loadFromFile(assetFilePath);
}

std::vector<std::filesystem::path> SoundAsset::getDependencies() const {
    return {
        { assetFilePath + assetName }
    };
}

}