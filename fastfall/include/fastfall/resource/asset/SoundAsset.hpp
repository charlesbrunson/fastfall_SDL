#pragma once

#include "fastfall/resource/Asset.hpp"
#include "soloud_wav.h"

namespace ff {

class SoundAsset : public Asset {
public:
    SoundAsset(const std::filesystem::path& t_asset_path);

    bool loadFromFile() override;
    bool reloadFromFile() override;

    std::vector<std::filesystem::path> getDependencies() const override;

    SoLoud::Wav& wav() { return sound; }
    const SoLoud::Wav& wav() const { return sound; }

    operator SoLoud::Wav&() { return wav(); }
    operator const SoLoud::Wav&() const { return wav(); }

    void ImGui_getContent(secs deltaTime) override {};

private:
    SoLoud::Wav sound;
};

}