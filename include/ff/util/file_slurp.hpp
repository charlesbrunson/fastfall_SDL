#pragma once

#include <memory>
#include <filesystem>

namespace ff {

std::unique_ptr<char[]> file_slurp(std::filesystem::path path);

}