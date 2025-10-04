#pragma once

#include "fastfall/resource/Asset.hpp"

struct SDL_IOStream;

namespace ff {

class MusicAsset : public Asset {
public:
    explicit MusicAsset(const std::filesystem::path& t_asset_path);
    ~MusicAsset() override;

    bool loadFromFile() override;
    bool reloadFromFile() override;

    [[nodiscard]] std::vector<std::filesystem::path> getDependencies() const override;

    void ImGui_getContent(secs deltaTime) override {};

    [[nodiscard]] SDL_IOStream* get_data() const { return music_impl; }

private:
    SDL_IOStream* music_impl = nullptr;

    void destroy_data_source();
};

}
