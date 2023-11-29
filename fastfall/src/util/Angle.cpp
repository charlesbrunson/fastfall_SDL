#include "fastfall/util/Angle.hpp"

using namespace ff;

const AngleRange Any = {
    .min = Angle::Degree(std::nextafterf(-180.f, 0.f)),
    .max = Angle::Degree(180.f),
    .inclusive = true
};