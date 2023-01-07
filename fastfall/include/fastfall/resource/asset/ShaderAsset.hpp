#pragma once

#include "fastfall/resource/Asset.hpp"
#include "fastfall/render/Shader.hpp"

namespace ff {

class ShaderAsset : public Asset {
public:
    ShaderAsset(std::string_view vertex_file,
                std::string_view fragment_file,
                const std::vector<std::string>& uniforms);

    bool loadFromFile(const std::string& relpath) override;
    bool reloadFromFile() override;

    ShaderProgram& getProgram() { return program; }

    std::vector<std::filesystem::path> getDependencies() const override {
        return {
            getFilePath() + vertex_file,
            getFilePath() + fragment_file,
        };
    }

private:
    ShaderProgram program;
    std::string vertex_file;
    std::string fragment_file;
};

}