#pragma once

#include "fastfall/resource/Asset.hpp"
#include "soloud_wavstream.h"

namespace ff {

class MusicAsset : public Asset {
public:
    MusicAsset(const std::filesystem::path& t_asset_path);

    bool loadFromFile() override;
    bool reloadFromFile() override;

    std::vector<std::filesystem::path> getDependencies() const override;

    SoLoud::WavStream& wavstream() { return stream; }
    const SoLoud::WavStream& wavstream() const { return stream; }


    void ImGui_getContent(secs deltaTime) override {};

private:
    SoLoud::WavStream stream;
};

}
