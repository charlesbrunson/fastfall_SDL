#pragma once

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

    [[nodiscard]] Impl* get_data() { return music_ptr; };
    [[nodiscard]] const Impl* get_data() const { return music_ptr; };

    void ImGui_getContent(secs deltaTime) override {};

private:
    Impl* music_ptr;
};

}
