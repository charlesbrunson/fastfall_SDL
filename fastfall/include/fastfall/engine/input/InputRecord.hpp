#pragma once

#include "fastfall/engine/time/time.hpp"

#include <bitset>
#include <vector>
#include <array>

namespace ff {

    constexpr size_t INPUT_RECORD_SIZE_MAX = 60 * 60 * 30; // 30 mins of record at 60 fps

    struct InputFrame {
        std::bitset<INPUT_COUNT> pressed;
        std::array<uint8_t, INPUT_COUNT> magnitudes;
    };

    struct InputRecord {
        secs deltaTime;
        std::set<InputType> listening;
        std::vector<InputFrame> frame_data;
    };

}