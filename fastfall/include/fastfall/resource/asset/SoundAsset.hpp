#pragma once

#include "fastfall/resource/Asset.hpp"

namespace ff {

class SoundAsset : public Asset {
public:
    struct Impl;

    explicit SoundAsset(const std::filesystem::path& t_asset_path);
    ~SoundAsset() override;

    bool loadFromFile() override;
    bool reloadFromFile() override;

    [[nodiscard]] std::vector<std::filesystem::path> getDependencies() const override;

    [[nodiscard]] Impl* get_data() { return sound_ptr; };
    [[nodiscard]] const Impl* get_data() const { return sound_ptr; };

    void ImGui_getContent(secs deltaTime) override {};

private:
    Impl* sound_ptr;
};

}