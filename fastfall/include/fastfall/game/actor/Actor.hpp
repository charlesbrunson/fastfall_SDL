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
    explicit Actor(ActorInit init)
        : entity_id     (init.entity_id)
        , actor_id      (init.actor_id)
        , priority      (init.get_priority())
        , type          (init.type)
        , level_object  (init.level_object)
    {
    };

    virtual ~Actor() = default;

    virtual void update(World& world, secs deltaTime) {};
    virtual void predraw(World& world, float interp, bool updated) {};
    virtual dresult message(World&, const dmessage&) { return reject; }
    virtual void ImGui_Inspect() {};

    const ID<Entity>    entity_id;
    const ID<Actor>     actor_id;
    const uint8_t       priority;

    [[nodiscard]] bool is_dead() const { return dead; }
    [[nodiscard]] bool is_initialized() const { return initialized; }

    bool             has_actor_type() const { return type; }
    const ActorType* get_actor_type() const { return type; }

    std::optional<ObjLevelID> level_id() const {
        return level_object ? std::make_optional(level_object->level_id) : std::nullopt;
    }

protected:
    bool initialized = true;
    bool dead = false;

    bool                   has_level_object() const { return level_object; }
    const LevelObjectData* get_level_object() const { return level_object; }

private:
    const ActorType*       const type         = nullptr;
    const LevelObjectData* const level_object = nullptr;
};

template<class T, class... Args>
concept valid_actor_ctor = std::derived_from<T, Actor> && std::constructible_from<T, ActorInit, Args...>;

}
