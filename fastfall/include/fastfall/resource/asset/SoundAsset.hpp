#pragma once

#include "miniaudio.h"
#include "fastfall/resource/Asset.hpp"

namespace ff {

class SoundAsset : public Asset {
public:
    explicit SoundAsset(const std::filesystem::path& t_asset_path);
    ~SoundAsset() override;

    bool loadFromFile() override;
    bool reloadFromFile() override;

    [[nodiscard]] std::vector<std::filesystem::path> getDependencies() const override;

    void ImGui_getContent(secs deltaTime) override {};

    ma_resource_manager_data_source* get_data_source() const { return data_source.get(); }

private:
    std::unique_ptr<ma_resource_manager_data_source> data_source = nullptr;

    void destroy_data_source();
};

}