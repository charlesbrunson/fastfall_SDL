#pragma once

#include <filesystem>

namespace ff {
    bool Init();
    bool Load_Resources(std::filesystem::path root);
    void Quit();
}