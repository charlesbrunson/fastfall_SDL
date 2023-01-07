#pragma once

#include "fastfall/resource/Asset.hpp"
#include "fastfall/render/Shader.hpp"

namespace ff {

class ShaderAsset : public Asset {
public:
    ShaderAsset(const std::string& assetName);

    bool loadFromFile(const std::string& relpath) override;
    bool reloadFromFile() override;

    ShaderProgram& getProgram() { return program; }

    bool compileShaderFromFile();

    std::vector<std::filesystem::path> getDependencies() const override {
        return {
            getFilePath() + getAssetName(),
            getFilePath() + vertex_file,
            getFilePath() + fragment_file,
        };
    }

    // TODO
    void ImGui_getContent() override {};

private:
    ShaderProgram program;
    std::string vertex_file;
    std::string fragment_file;
    std::vector<std::string> uniforms;
};

}