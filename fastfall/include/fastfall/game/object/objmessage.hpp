#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/dmessage.hpp"

namespace ff {

class World;

// variant of all valid parameter types
using objvar = std::variant<
    std::monostate,
    bool,
    int,
    float,
    Vec2i,
    Vec2f
>;

using objcfg = dconfig<objvar, World&>;

static constexpr auto objNoOp   = objcfg::dformat<"noop">{};
static constexpr auto objGetPos = objcfg::dformat<"getpos", Vec2f>{};
static constexpr auto objSetPos = objcfg::dformat<"getpos", std::monostate, Vec2f>{};
static constexpr auto objHurt   = objcfg::dformat<"hurt", std::monostate, float>{};

}