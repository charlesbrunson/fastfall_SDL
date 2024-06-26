#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/input/Input_Def.hpp"

#include <bitset>
#include <vector>
#include <array>
//#include <set>
#include <filesystem>

namespace ff {

    constexpr size_t INPUT_RECORD_SIZE_MAX = 60 * 60 * 30; // 30 mins of record at 60 fps

    struct InputFrame {
        uint8_t pressed             = {};
        uint8_t activation_change   = {};
        std::array<uint8_t, INPUT_COUNT> magnitudes = {};

        std::string to_string() const;
    };

    struct InputRecord {
        secs                    deltaTime;
        uint8_t                 listening;
        std::vector<InputFrame> frame_data;

        bool save_to_file(  std::filesystem::path path);
        bool load_from_file(std::filesystem::path path);
    };

}