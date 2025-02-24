#pragma once

#include "miniaudio.h"
#include "fastfall/resource/Asset.hpp"

namespace ff {

struct ma_sound_deleter
{
    void operator()(ma_sound* sound);
};

class SoundAsset : public Asset {
public:
    struct Impl;

    explicit SoundAsset(const std::filesystem::path& t_asset_path);

    bool loadFromFile() override;
    bool reloadFromFile() override;

    [[nodiscard]] std::vector<std::filesystem::path> getDependencies() const override;

    void ImGui_getContent(secs deltaTime) override {};

    ma_sound& get_sound() { return *sound; }

private:
    std::unique_ptr<ma_sound, ma_sound_deleter> sound = nullptr;
};

}