#pragma once

#include "fastfall/resource/asset/AnimAssetTypes.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/render/util/Color.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/id.hpp"

#include <variant>

namespace ff {

class World;
class Entity;
class Actor;
struct ActorType;

struct ActorInit {
    World&      world;
    ID<Entity>  entity_id;
    ID<Actor>   actor_id;
    uint8_t     priority = 0;

    const ActorType*       const type         = nullptr;
    const LevelObjectData* const level_object = nullptr;
};

enum class ActorPropertyType : size_t {
    String  = 0,
    Int     = 1,
    Bool    = 2,
    Float   = 3,
    Object  = 4,
};

using ActorPropertyValue = std::variant<
    std::string,
    int,
    bool,
    float,
    ObjLevelID
>;

struct ActorProperty
{
    // defines type with a default value
    template<typename T>
        requires std::is_constructible_v<ActorPropertyValue, T>
    constexpr ActorProperty(std::string_view t_name, T t_value)
        : name(t_name)
        , value(t_value)
    {
        type = static_cast<ActorPropertyType>(value->index());
    }

    // defines a type, but requires a value
    ActorProperty(std::string_view t_name, ActorPropertyType t_type)
        : name(t_name)
        , type(t_type)
    {
    }

    std::string name;
    ActorPropertyType type;
    std::optional<ActorPropertyValue> value = std::nullopt;
};

struct ActorType {
    // name and type hash
    struct Name {
        Name(std::string_view _str)
                : str(_str)
                , hash(std::hash<std::string_view>{}(str))
        {
        }

        const std::string str;
        const size_t hash;
    } const name;

    const std::optional<AnimIDRef> anim = std::nullopt;
    const Vec2u tile_size               = { 0u, 0u };
    const Color tile_fill_color         = Color::White().alpha(128u);

    const std::vector<ActorGroupTag> group_tags = {};
    const std::vector<ActorProperty> properties = {};

    std::function<Actor(ActorInit, const LevelObjectData&)> builder;
};

}