#pragma once

#include "fastfall/resource/asset/AnimAssetTypes.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/render/util/Color.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"

#include <variant>
#include <functional>
#include <utility>
#include <tuple>

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
    ActorProperty(std::string t_name, T t_value)
        : name(std::move(t_name))
        , value(t_value)
    {
        type = static_cast<ActorPropertyType>(value->index());
    }

    // defines a type, but requires a value
    ActorProperty(std::string t_name, ActorPropertyType t_type)
        : name(std::move(t_name))
        , type(t_type)
    {
    }

    std::string name;
    ActorPropertyType type;
    std::optional<ActorPropertyValue> value = std::nullopt;
};

using level_data_builder_fn = copyable_unique_ptr<Actor>(ActorInit, const LevelObjectData&);

template<typename Fn, typename ActorType>
concept converts_level_data_for_actor = requires (Fn fn, ActorInit init, const LevelObjectData& data) {
    { std::make_from_tuple<ActorType>( std::tuple_cat(std::make_tuple(init), fn(data)) ) } -> std::same_as<ActorType>;
};

template<std::derived_from<Actor> T>
std::function<level_data_builder_fn>
make_level_data_parser(converts_level_data_for_actor<T> auto&& fn)
{
    return [&](ActorInit init, const LevelObjectData& data) {
        return std::apply(
            [](auto&&... args) {
                return make_copyable_unique<Actor, T>( std::forward<decltype(args)>(args)... );
            },
            std::tuple_cat(
                std::make_tuple(init),
                fn(data)
            )
        );
    };
}

template<std::derived_from<Actor> T>
std::function<level_data_builder_fn>
make_level_data_parser()
{
    return [](ActorInit init, const LevelObjectData&) {
        return make_copyable_unique<Actor, T>(init);
    };
}

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
    const uint8_t priority;

    const std::vector<ActorGroupTag> group_tags = {};
    const std::vector<ActorProperty> properties = {};

    const std::function<level_data_builder_fn> builder = nullptr;
    copyable_unique_ptr<Actor> make_with_data(ActorInit) const;
};

}