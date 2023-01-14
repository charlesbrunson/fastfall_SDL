#include "fastfall/resource/asset/ShaderAsset.hpp"
#include "fastfall/util/log.hpp"

#include <fstream>

#include "fastfall/util/xml.hpp"
#include "rapidxml.hpp"
using namespace rapidxml;

namespace ff {

ShaderAsset::ShaderAsset(const std::filesystem::path& t_asset_path)
    : Asset(t_asset_path)
{
}

bool ShaderAsset::loadFromFile() {

    program = {};
    uniforms.clear();
    vertex_path.clear();
    fragment_path.clear();

    bool r = true;
    std::unique_ptr<char[]> charPtr = readXML(asset_path);
    if (charPtr) {
        char *xmlContent = charPtr.get();
        auto doc = std::make_unique<xml_document<>>();

        try {
            doc->parse<0>(xmlContent);
            xml_node<>* index = doc->first_node("shader");

            if (!index)
                throw parse_error("no shader", nullptr);

            xml_node<>* node = index->first_node();
            while (node) {
                if (strcmp("links", node->name()) == 0) {
                    xml_node<>* link = node->first_node("link");
                    while(link) {
                        if (auto* ptr = link->first_attribute("file")) {
                            std::string_view file = ptr->value();
                            if (file.ends_with(".vert")) {
                                vertex_path = asset_path.parent_path().concat(file);
                            }
                            else if (file.ends_with(".frag")) {
                                vertex_path = asset_path.parent_path().concat(file);
                            }
                            else {
                                throw parse_error("unable to determine shader link type", nullptr);
                            }
                        }
                        else {
                            throw parse_error("link has no file attr", nullptr);
                        }
                        link = link->next_sibling("link");
                    }
                }
                else if (strcmp("uniforms", node->name()) == 0) {
                    xml_node<>* link = node->first_node("uniform");
                    while(link) {
                        if (auto* ptr = link->first_attribute("name")) {
                            uniforms.emplace_back(ptr->value());
                        }
                        else {
                            throw parse_error("uniform has no name attr", nullptr);
                        }
                        link = link->next_sibling("uniform");
                    }
                }
                node = node->next_sibling();
            }
        }
        catch (parse_error& err) {
            std::cout << asset_path << ": " << err.what() << std::endl;
            r = false;
        }
    }
    else {
        std::cout << "Could not open file: " << asset_path << std::endl;
        r = false;
    }

    r &= !vertex_path.empty();
    r &= !fragment_path.empty();
    loaded = r;
    return r;
}

bool ShaderAsset::reloadFromFile() {
    try {
        ShaderAsset nasset{ asset_path };
        bool r = nasset.loadFromFile() && nasset.compileShaderFromFile();
        if (r) {
            *this = std::move(nasset);
        }
        return r;
    }
    catch(std::exception& e) {

    }
    return false;
}

bool ShaderAsset::compileShaderFromFile() {

    program = ShaderProgram{};

    auto load_shader = [&](const std::filesystem::path& file_path, ShaderType type) {
        std::stringstream stream;
        std::ifstream file{ file_path };
        if (file.is_open()) {
            stream << ShaderProgram::getGLSLVersionString();
            stream << file.rdbuf();
            program.add(type, stream.str());
            return true;
        } else {
            LOG_ERR_("{}: unable to compile, {} not found", asset_path.c_str(), file_path.c_str());
            program = ShaderProgram{};
            return false;
        }
    };

    if (!load_shader(vertex_path, ShaderType::VERTEX)) { return false; }
    if (!load_shader(fragment_path, ShaderType::FRAGMENT)) { return false; }

    if (program.isInitialized()) {
        program.link();

        for (auto &uniform: uniforms) {
            program.cacheUniform(uniform);
        }
    }
    return program.isLinked();
}

}

