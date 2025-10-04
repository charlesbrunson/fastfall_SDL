#pragma once

#include "fastfall/resource/Asset.hpp"

struct MIX_Audio;

namespace ff {

class SoundAsset : public Asset {
public:
    explicit SoundAsset(const std::filesystem::path& t_asset_path);
    ~SoundAsset() override;

    bool loadFromFile() override;
    bool reloadFromFile() override;

    [[nodiscard]] std::vector<std::filesystem::path> getDependencies() const override;

    void ImGui_getContent(secs deltaTime) override {};

    [[nodiscard]] MIX_Audio* get_data() const { return audio_impl; }

private:
    MIX_Audio* audio_impl = nullptr;
};

}