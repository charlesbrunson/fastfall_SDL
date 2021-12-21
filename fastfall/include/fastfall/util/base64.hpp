#pragma once

#include <vector>
#include <string>

namespace ff {

//std::string base64_encode(BYTE const* buf, unsigned int bufLen);
std::vector<uint8_t> base64_decode(std::string const& str);

std::string base64_encode(uint8_t const* buf, unsigned int len);

}