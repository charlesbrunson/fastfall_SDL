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

class  World;
class  Entity;
class  Actor;
struct ActorType;

struct ActorInit {
    World&      world;
    ID<Entity>  entity_id;
    ID<Actor>   actor_id;

    const ActorType* type = nullptr;
    const LevelObjectData* const level_object = nullptr;

    uint8_t get_priority() const;
    ActorInit& type_or(const ActorType* n_type);
    copyable_unique_ptr<Actor> create() const;
};

struct ActorProperty
{
    // defines type with a default value
    ActorProperty(std::string_view t_name, ObjectProperty::Variant t_value)
        : name(t_name)
        , value(std::move(t_value))
    {
        type = static_cast<ObjectProperty::Type>(value->index());
    }

    // defines a type, but requires a value
    ActorProperty(std::string_view t_name, ObjectProperty::Type t_type)
        : name(t_name)
        , type(t_type)
    {
    }

    std::string name;
    ObjectProperty::Type type;
    std::optional<ObjectProperty::Variant> value = std::nullopt;
};

struct ActorTypeInfo {
    struct Name {
        Name(std::string_view _str)
                : str(_str), hash(std::hash<std::string_view>{}(_str))
        {}

        Name(const char* _str)
                : Name(std::string_view{ _str })
        {}

        const std::string str;
        const size_t hash;
    } const name;

    constexpr inline static uint8_t priority_default = 128;

    const AnimIDRef anim = {};
    const Vec2u   tile_size       = { 0u, 0u };
    const Color   tile_fill_color = Color::White().alpha(128u);
    const uint8_t priority = priority_default;

    const std::vector<ActorGroupTag> group_tags = {};
    const std::vector<ActorProperty> properties = {};
};

class ActorType : public ActorTypeInfo {
    explicit ActorType(const ActorTypeInfo& info) : ActorTypeInfo(info) {}

    using builder_fn = copyable_unique_ptr<Actor>(*)(ActorInit, const LevelObjectData&);
    builder_fn builder = nullptr;
public:

    template <std::derived_from<Actor> T>
    static ActorType create(ActorTypeInfo info)
    {
        auto type = ActorType{ info };
        if constexpr (std::is_constructible_v<T, ActorInit, const LevelObjectData&>) {
            type.builder = [](ActorInit init, const LevelObjectData &data) -> copyable_unique_ptr<Actor> {
                return make_copyable_unique<Actor, T>(init, data);
            };
        }
        return type;
    }

    bool has_builder() const {
        return builder;
    }

    copyable_unique_ptr<Actor> build_with_data(ActorInit init, const LevelObjectData &data) const {
        return builder ? builder(init, data) : copyable_unique_ptr<Actor>{};
    }

};

}