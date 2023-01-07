#include "fastfall/resource/asset/ShaderAsset.hpp"
#include "fastfall/util/log.hpp"

#include <fstream>

#include "fastfall/util/xml.hpp"
#include "rapidxml.hpp"
using namespace rapidxml;

namespace ff {

ShaderAsset::ShaderAsset(const std::string& assetName)
    : Asset(assetName)
{
}

bool ShaderAsset::loadFromFile(const std::string& relpath) {

    program = {};
    uniforms.clear();
    vertex_file.clear();
    fragment_file.clear();

    bool r = true;
    assetFilePath = relpath;
    std::unique_ptr<char[]> charPtr = readXML(assetFilePath + assetName);
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
                                vertex_file = file;
                            }
                            else if (file.ends_with(".frag")) {
                                fragment_file = file;
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
            std::cout << assetName << ": " << err.what() << std::endl;
            r = false;
        }
    }
    else {
        std::cout << "Could not open file: " << relpath + assetName << std::endl;
        r = false;
    }

    r &= !vertex_file.empty();
    r &= !fragment_file.empty();
    loaded = r;
    return r;
}

bool ShaderAsset::reloadFromFile() {
    return loadFromFile(assetFilePath);
}

bool ShaderAsset::compileShaderFromFile() {
    program = ShaderProgram{};
    {
        std::stringstream stream;
        std::ifstream file{assetFilePath + vertex_file};
        if (file.is_open()) {
            stream << ShaderProgram::getGLSLVersionString();
            stream << file.rdbuf();
            program.add(ShaderType::VERTEX, stream.str());
        } else {
            LOG_ERR_("{}: unable to compile, {} not found", assetFilePath + assetName, assetFilePath + vertex_file);
            program = ShaderProgram{};
            return false;
        }
    }

    {
        std::stringstream stream;
        std::ifstream file{assetFilePath + fragment_file};
        if (file.is_open()) {
            stream << ShaderProgram::getGLSLVersionString();
            stream << file.rdbuf();
            program.add(ShaderType::FRAGMENT, stream.str());
        } else {
            LOG_ERR_("{}: unable to compile, {} not found", assetFilePath + assetName, assetFilePath + vertex_file);
            program = ShaderProgram{};
            return false;
        }
    }

    if (program.isInitialized()) {
        program.link();

        for (auto &uniform: uniforms) {
            program.cacheUniform(uniform);
        }
    }
    return program.isLinked();
}

}

