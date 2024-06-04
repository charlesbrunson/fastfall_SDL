#pragma once

#include <string>
#include <string_view>
#include <span>

namespace ff {

std::string base64_decode(std::string_view str);
std::string base64_encode(std::span<uint8_t> data);

}