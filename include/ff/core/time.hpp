#pragma once

namespace ff {

using seconds = double;

struct tick_info {
    unsigned update_count;
    float    interp;
    seconds  deltatime;
};

}