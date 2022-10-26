#include "fastfall/engine/input/InputRecord.hpp"

#include "fmt/format.h"

using namespace ff;

std::string InputFrame::to_string() const {
    return fmt::format(
        "{:08b} {:08b} {:3d} {:3d} {:3d} {:3d} {:3d} {:3d} {:3d}",
        pressed.to_ulong(),
        activation_change.to_ulong(),
        magnitudes[0],
        magnitudes[1],
        magnitudes[2],
        magnitudes[3],
        magnitudes[4],
        magnitudes[5],
        magnitudes[6]
    );
}
