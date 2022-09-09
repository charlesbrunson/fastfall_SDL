#pragma once

namespace ff {

struct QuadID {
    int value = -1;

    bool operator==(const QuadID other) const {
        return value == other.value;
    }
    bool operator<(const QuadID other) const {
        return value < other.value;
    }
};

}

