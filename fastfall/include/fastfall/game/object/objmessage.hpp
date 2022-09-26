#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/dmessage.hpp"

namespace ff {
    class World;

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
    static constexpr auto objHurt   = objcfg::dformat<"hurt", std::monostate, float>{};
}