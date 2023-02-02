#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/dmessage.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/game/ComponentID.hpp"

namespace ff {

class World;
class Entity;

using actor_vars = std::variant<
    dvoid,
    bool,
    int,
    float,
    Vec2i,
    Vec2f,
    secs,
    ComponentID,
    ID<Entity>
>;

class Actor;

struct ActorInit {
    World& world;
    ID<Entity> entity_id;
    ID<Actor> actor_id;
};

class Actor : public dconfig<actor_vars, World&>
{
public:
    explicit Actor(ActorInit init)
        : entity_id(init.entity_id)
        , actor_id(init.actor_id)
    {
    };

    virtual ~Actor() = default;

    virtual void update(World& world, secs deltaTime) {};
    virtual dresult message(World&, const dmessage&) { return reject; }

    const ID<Entity> entity_id;
    const ID<Actor>  actor_id;

    [[nodiscard]]
    bool is_dead() const { return dead; }

protected:
    bool initialized = true;
    bool dead = false;

private:
    friend class World;
};

namespace actor_msg {
    static constexpr auto NoOp   = Actor::dformat<"noop">{};
    static constexpr auto GetPos = Actor::dformat<"getpos", Vec2f>{};
    static constexpr auto SetPos = Actor::dformat<"setpos", dvoid, Vec2f>{};
    static constexpr auto Hurt   = Actor::dformat<"hurt", dvoid, float>{};
}

template<class T, class... Args>
concept valid_actor_ctor = std::derived_from<T, Actor> && std::constructible_from<T, ActorInit, Args...>;

}
