#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/dmessage.hpp"
#include "fastfall/util/math.hpp"

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

class Actor : public dconfig<actor_vars, World&>
{
public:
    explicit Actor(ID<Entity> actor_entity_id)
        : entity_id(actor_entity_id)
    {
    };

    virtual ~Actor() = default;

    virtual void update(World& world, secs deltaTime) {};
    virtual dresult message(World&, const dmessage&) { return reject; }

    const ID<Entity> entity_id;

protected:
    virtual bool init_entity(World&) = 0;

    friend class World;
};

template<class T, class... Args>
concept valid_actor_ctor = std::derived_from<T, Actor> && requires(World& w, ID<Entity> id, Args&&... args) {
    T{ w, id, std::forward<Args>(args)... };
};

}
