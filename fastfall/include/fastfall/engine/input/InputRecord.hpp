#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/input/Input_Def.hpp"

#include <bitset>
#include <vector>
#include <array>
#include <set>

namespace ff {

    constexpr size_t INPUT_RECORD_SIZE_MAX = 60 * 60 * 30; // 30 mins of record at 60 fps

    struct InputFrame {
        std::bitset<INPUT_COUNT> pressed;
        std::bitset<INPUT_COUNT> activation_change;
        std::array<uint8_t, INPUT_COUNT> magnitudes;

        std::string to_string() const;
    };

    struct InputRecord {
        secs deltaTime;
        std::set<InputType> listening;
        std::vector<InputFrame> frame_data;
    };

}