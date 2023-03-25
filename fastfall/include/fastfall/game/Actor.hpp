#pragma once

#include "imgui.h"
#include "ActorMsg.hpp"
#include "ActorType.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"

namespace ff {

class World;
class Entity;
class Actor;
class ActorType;


class Actor : public actor_dconfig
{
public:
    explicit Actor(ActorInit init, const std::string& type_name)
        : entity_id(init.entity_id)
        , actor_id (init.actor_id)
        , priority (init.priority)
        , actor_type(type_name)
    {
    };

    virtual ~Actor() = default;

    virtual void update(World& world, secs deltaTime) {};
    virtual void predraw(World& world, float interp, bool updated) {};
    virtual dresult message(World&, const dmessage&) { return reject; }
    virtual void ImGui_Inspect() {};

    const std::string   actor_type;
    const ID<Entity>    entity_id;
    const ID<Actor>     actor_id;
    const uint8_t priority;

    [[nodiscard]] bool is_dead() const { return dead; }
    [[nodiscard]] bool is_initialized() const { return initialized; }

protected:
    bool initialized = true;
    bool dead = false;

};

template<class T, class... Args>
concept valid_actor_ctor = std::derived_from<T, Actor> && std::constructible_from<T, ActorInit, Args...>;

}
