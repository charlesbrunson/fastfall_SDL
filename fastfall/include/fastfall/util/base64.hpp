#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace ff {

std::string base64_decode(std::string const& str);
std::string base64_encode(uint8_t const* buf, unsigned int len);

}