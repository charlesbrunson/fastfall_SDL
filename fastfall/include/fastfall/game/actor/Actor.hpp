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

    bool has_any_type() const { return type != nullptr; }

    template<typename T_Actor>
    bool has_type() const {
        return type != nullptr && type == actor_type_of<T_Actor>();
    }

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

template<class T>
//concept actor_has_type = std::same_as<typename T::actor_type, const ActorType>;
concept actor_has_type = requires (T t) {
    { T::actor_type } -> std::same_as<const ActorType&>;
};

template<std::derived_from<Actor> T>
const ActorType* actor_type_of() {
    if constexpr (actor_has_type<T>) {
        return &T::actor_type;
    } else {
        return nullptr;
    }
}

/*
template<std::derived_from<Actor> T>
constexpr inline static const ActorType* actor_type_of_v = []() {
    if constexpr (actor_has_type<T>) {
        return &T::actor_type;
    } else {
        return nullptr;
    }
}();
*/

}
