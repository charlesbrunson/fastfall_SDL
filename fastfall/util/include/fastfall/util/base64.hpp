#pragma once

#include <vector>
#include <string>

namespace ff {

//std::string base64_encode(BYTE const* buf, unsigned int bufLen);
std::vector<int8_t> base64_decode(std::string const&);

}