#include "fastfall/resource/asset/MusicAsset.hpp"

namespace ff {

MusicAsset::MusicAsset(const std::filesystem::path &t_asset_path)
    : Asset(t_asset_path)
{
}

bool MusicAsset::loadFromFile() {
    auto str = asset_path.generic_string();
    // TODO: replace SoLoud
    // loaded = stream.load(str.data()) == SoLoud::SO_NO_ERROR;
    loaded = true;
    return loaded;
}

bool MusicAsset::reloadFromFile() {
    return true;
}

std::vector<std::filesystem::path> MusicAsset::getDependencies() const {
    return { asset_path };
}

}