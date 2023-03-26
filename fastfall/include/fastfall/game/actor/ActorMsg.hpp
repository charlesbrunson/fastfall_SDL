#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/dmessage.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"
#include "fastfall/util/id.hpp"
#include <variant>

namespace ff {

    class Entity;
    class World;

    // 16 max
    using actor_vars = std::variant<
        dvoid,
        bool,
        int,
        float,
        Vec2i,
        Vec2f,
        secs,
        ID<Entity>,
        AnimID
    >;

    using actor_dconfig = dconfig<actor_vars, World&>;

    namespace actor_msg {
        static constexpr auto NoOp   = actor_dconfig::dformat<"noop">{};
        static constexpr auto GetPos = actor_dconfig::dformat<"getpos", Vec2f>{};
        static constexpr auto SetPos = actor_dconfig::dformat<"setpos", dvoid, Vec2f>{};
        static constexpr auto Hurt   = actor_dconfig::dformat<"hurt",   dvoid, float>{};
    }
}