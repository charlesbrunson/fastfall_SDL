#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/dmessage.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/game/ComponentID.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include "imgui.h"

namespace ff {

class World;
class Entity;

// 16 max
using actor_vars = std::variant<
    dvoid,
    bool,
    int,
    float,
    Vec2i,
    Vec2f,
    secs,
    ComponentID,
    ID<Entity>,
    AnimID
>;

class Actor;

enum class ActorType {
    Level   = 0,
    Object  = 1,
    Actor   = 2
};

enum class ActorPriority {
    Highest     = 0,
    High        = 1,
    Normal      = 2,
    Low         = 3,
    Lowest      = 4
};

class ObjectType;

struct ActorInit {
    World&        world;
    ID<Entity>    entity_id;
    ID<Actor>     actor_id;
    ActorType     type;
    ActorPriority priority;
};


class Actor : public dconfig<actor_vars, World&>
{
public:
    explicit Actor(ActorInit init, const std::string& type_name)
        : entity_id(init.entity_id)
        , actor_id (init.actor_id)
        , type     (init.type)
        , priority (init.priority)
        , actor_type(type_name)
    {
    };
    virtual ~Actor() = default;

    virtual void update(World& world, secs deltaTime) {};
    virtual dresult message(World&, const dmessage&) { return reject; }
    virtual void ImGui_Inspect() {};

    const std::string   actor_type;
    const ID<Entity>    entity_id;
    const ID<Actor>     actor_id;
    const ActorType     type;
    const ActorPriority priority;

    [[nodiscard]]
    bool is_dead() const { return dead; }

protected:
    bool initialized = true;
    bool dead = false;

private:
    friend class World;
};

inline auto actor_compare(const Actor &lhs, const Actor &rhs) {
    auto type_cmp = lhs.type <=> rhs.type;
    return type_cmp == std::strong_ordering::equal
           ? lhs.priority <=> rhs.priority
           : type_cmp;
}

namespace actor_msg {
    static constexpr auto NoOp   = Actor::dformat<"noop">{};
    static constexpr auto GetPos = Actor::dformat<"getpos", Vec2f>{};
    static constexpr auto SetPos = Actor::dformat<"setpos", dvoid, Vec2f>{};
    static constexpr auto Hurt   = Actor::dformat<"hurt", dvoid, float>{};
}

template<class T, class... Args>
concept valid_actor_ctor = std::derived_from<T, Actor> && std::constructible_from<T, ActorInit, Args...>;

}
