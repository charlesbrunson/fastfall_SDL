#include "fastfall/resource/asset/SoundAsset.hpp"

#include <fastfall/engine/audio.hpp>
#include <memory>

namespace ff {

SoundAsset::SoundAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

SoundAsset::~SoundAsset() {
    destroy_data_source();
}


void SoundAsset::destroy_data_source() {
    if (audio::is_init() && data_source) {
        ma_resource_manager_data_source_uninit(data_source.get());
        data_source.reset();
    }
}

bool SoundAsset::loadFromFile() {
    auto str = asset_path.generic_string();

    destroy_data_source();

    loaded = false;
    if (audio::is_init())
    {
        data_source = std::make_unique<ma_resource_manager_data_source>();

        loaded = ma_resource_manager_data_source_init(
            audio::get_engine().pResourceManager,
            str.c_str(),
            MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE,
            nullptr,
            data_source.get()) == MA_SUCCESS;

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
