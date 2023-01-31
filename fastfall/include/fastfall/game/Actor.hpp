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
    enum Type {
        GameObject,
        Level
    };

    explicit Actor(Type type) : _type(type) {};
    virtual ~Actor() = default;

    virtual bool init(World& world, ID<Entity> entity) = 0;
    virtual void update(World& world, ID<Entity> entity, secs deltaTime) = 0;
    virtual void notify_active_level_reloaded(World& world) {};
    virtual dresult message(World&, const dmessage&) { return reject; }

    [[nodiscard]]
    Type type() const { return _type; }

private:
    Type _type;
};

}
