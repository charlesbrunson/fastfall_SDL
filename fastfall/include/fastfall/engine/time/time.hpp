#pragma once

#include <cmath>


typedef double secs;

constexpr secs ms_to_secs(size_t ms) noexcept {
    return 0.001 * ms;
}

namespace ff {

struct predraw_state_t {
    float interp;
    bool  updated;
    secs  update_dt;
};

}

