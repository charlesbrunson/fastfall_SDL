#pragma once

#include "fastfall/resource/Asset.hpp"
#include "fastfall/render/Shader.hpp"

namespace ff {

class ShaderAsset : public Asset {
public:
    ShaderAsset(const std::filesystem::path& t_asset_path);

    bool loadFromFile() override;
    bool reloadFromFile() override;

    ShaderProgram& getProgram() { return program; }

    bool compileShaderFromFile();

    std::vector<std::filesystem::path> getDependencies() const override {
        return {
            asset_path,
            vertex_path,
            fragment_path,
        };
    }

    // TODO
    void ImGui_getContent() override {};

private:
    ShaderProgram program;
    std::filesystem::path vertex_path;
    std::filesystem::path fragment_path;
    std::vector<std::string> uniforms;
};

}