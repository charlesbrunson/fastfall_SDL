#include "fastfall/resource/asset/ShaderAsset.hpp"
#include "fastfall/util/log.hpp"

#include <fstream>

#include "fastfall/util/xml.hpp"
#include "rapidxml.hpp"
#include "../../render/detail/error.hpp"

using namespace rapidxml;

namespace ff {

ShaderAsset::ShaderAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
    asset_name = t_asset_path.stem().concat(".glsl").generic_string();
}

bool ShaderAsset::loadFromFile()
{
    program = {};
    uniforms.clear();

    bool r = true;
    vertex_path   = asset_path;
    fragment_path = asset_path.parent_path() / asset_path.stem().concat(".frag");

    r &= !vertex_path.empty();
    r &= !fragment_path.empty();
    loaded = r;
    return r;
}

bool ShaderAsset::reloadFromFile()
{
    try {
        ShaderAsset nasset{ asset_path };
        bool r = nasset.loadFromFile();
        if (r) {
            *this = std::move(nasset);
        }
        return r;
    }
    catch(std::exception& e) {

    }
    return false;
}

bool ShaderAsset::compileShaderFromFile()
{
    program = ShaderProgram{};

    log::scope sc;
    auto load_shader = [&](const std::filesystem::path& file_path, ShaderType type) {
        std::stringstream stream;
        std::ifstream file{ file_path };
        if (file.is_open()) {
            stream << ShaderProgram::getGLSLVersionString();
            stream << file.rdbuf();
            program.add(type, stream.str());
            return true;
        } else {
            program = ShaderProgram{};
            LOG_ERR_("{}: unable to compile, {} not found", asset_name, file_path.generic_string());
            return false;
        }
    };

    if (!load_shader(vertex_path, ShaderType::VERTEX)) { return false; }
    if (!load_shader(fragment_path, ShaderType::FRAGMENT)) { return false; }

    if (program.isInitialized()) {
        try {
            program.link();
        }
        catch (Error& e) {
            LOG_ERR_("{}", e.what())
        }

        if (program.isLinked()) {
            LOG_INFO("Shader compiled", asset_name);
            LOG_INFO("Attributes:");
            for (const auto& attr : program.all_attributes()) {
                log::scope sc1;
                LOG_INFO("id:{:2} loc:{:2} - {:20} \"{}\"", attr.id, attr.loc, uniformTypeEnumToString(attr.type), attr.name);
            }
            LOG_INFO("Uniforms:");
            for (const auto& uni : program.all_uniforms()) {
                log::scope sc1;
                LOG_INFO("id:{:2} loc:{:2} - {:20} \"{}\"", uni.id, uni.loc, uniformTypeEnumToString(uni.type), uni.name);
            }
        }
    }
    else {
        LOG_ERR_("failed to initialize", asset_name);
    }

    return program.isLinked();
}

}

