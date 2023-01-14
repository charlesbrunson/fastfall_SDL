#pragma once

#include "fastfall/resource/Asset.hpp"

#include "soloud_wav.h"

namespace ff {

class SoundAsset : public Asset {
public:
    SoundAsset(const std::string& filename);

    bool loadFromFile(const std::string& relpath) override;
    bool reloadFromFile() override;

    std::vector<std::filesystem::path> getDependencies() const override;

    SoLoud::Wav* wav() { return (loaded ? &sound : nullptr); }
    const SoLoud::Wav* wav() const { return (loaded ? &sound : nullptr); }

    void ImGui_getContent() override {};

private:
    SoLoud::Wav sound;
};

}