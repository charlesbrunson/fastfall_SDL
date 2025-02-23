#pragma once

#include "miniaudio.h"
#include "fastfall/resource/Asset.hpp"

namespace ff {

class MusicAsset : public Asset {
public:
    struct Impl;

    explicit MusicAsset(const std::filesystem::path& t_asset_path);
    ~MusicAsset() override;

    bool loadFromFile() override;
    bool reloadFromFile() override;

    [[nodiscard]] std::vector<std::filesystem::path> getDependencies() const override;

    void ImGui_getContent(secs deltaTime) override {};

private:
    ma_sound music;
};

}
