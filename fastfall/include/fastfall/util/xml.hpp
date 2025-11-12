#pragma once


#include <rapidxml/rapidxml.hpp>
using namespace rapidxml;

#include <memory>
#include <filesystem>

namespace ff {

std::unique_ptr<char[]> readXML(std::filesystem::path path);

}