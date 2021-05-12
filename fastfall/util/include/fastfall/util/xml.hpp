#pragma once


#include <rapidxml.hpp>
using namespace rapidxml;

#include <memory>
#include <string>

namespace ff {

std::unique_ptr<char[]> readXML(std::string path);

}