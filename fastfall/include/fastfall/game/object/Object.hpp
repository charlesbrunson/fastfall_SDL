#pragma once

#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include <functional>
#include <type_traits>
#include <set>
#include <string>
#include <typeindex>
#include <utility>

#include "imgui.h"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"
//#include "fastfall/game/object/objmessage.hpp"
#include "fastfall/game/Entity.hpp"


namespace ff {

class World;

//class GameObject;

enum class ObjectPropertyType {
	String,
	Int,
	Bool,
	Float,
	Object,
};

struct ObjectProperty {

	// only specify the expected type
	ObjectProperty(std::string propName, ObjectPropertyType type)
        : name(std::move(propName))
        , type(type)
        , default_value()
    {}

	// specify type and default value
    ObjectProperty(std::string propName, std::string value_default)
        : name(std::move(propName))
        , type(ObjectPropertyType::String)
        , default_value(std::move(value_default))
    {}

    ObjectProperty(std::string propName, int value_default)
        : name(std::move(propName)), type(ObjectPropertyType::Int)
        , default_value(std::to_string(value_default))
    {}

    ObjectProperty(std::string propName, bool value_default)
        : name(std::move(propName))
        , type(ObjectPropertyType::Bool)
        , default_value(std::to_string(value_default))
    {}

    ObjectProperty(std::string propName, float value_default)
        : name(std::move(propName))
        , type(ObjectPropertyType::Float)
        , default_value(std::to_string(value_default))
    {}

    ObjectProperty(std::string propName, ObjLevelID value_default)
        : name(std::move(propName))
        , type(ObjectPropertyType::Object)
        , default_value(std::to_string(value_default.id))
    {}

	std::string name;
	ObjectPropertyType type;
	std::string default_value;

    constexpr inline bool operator< (const ObjectProperty& rhs) const {
		return name < rhs.name;
	}
};

struct ObjectType
{
    // name and type hash
    struct Name {
        Name(std::string_view _str)
            : str(_str)
            , hash(std::hash<std::string_view>{}(str))
        {
        }

        const std::string_view str;
        const size_t hash;
    } const name;

    // appearance in editor
	const std::optional<AnimIDRef> anim = std::nullopt;
    const Vec2u tile_size               = { 0u, 0u };
    const Color tile_fill_color         = Color::White().alpha(128u);

    // update priority
    const ActorPriority priority = ActorPriority::Normal;

    // groups this type is part of
    const std::vector<ObjectGroupTag> group_tags = {};

    // properties optional/required init arguments from ObjectLevelData
    const std::vector<ObjectProperty> properties = {};

    bool test(ObjectLevelData& data) const;
};

class Object;

template<typename T>
concept valid_object = std::derived_from<T, Object> && std::same_as<decltype(T::Type), const ObjectType>;

struct ObjectFactory {
private:

    struct ObjectBuilder {
        using builderfn_t = copyable_unique_ptr<Actor>(*)(ActorInit, ObjectLevelData&);

        const ObjectType* const type;
        const builderfn_t create;
    };

	static std::unordered_map<size_t, ObjectBuilder> object_builders;

public:
	template<valid_object T>
	static void register_object()
    {
        object_builders.emplace(T::Type.name.hash, ObjectBuilder{
            .type = &T::Type,
            .create = [](ActorInit init, ObjectLevelData& data) -> copyable_unique_ptr<Actor>
            {
                if constexpr (std::is_constructible_v<T, ActorInit, ObjectLevelData&>) {
                    copyable_unique_ptr<Actor> ret;
                    init.object_type = &T::Type;
                    init.priority    = T::Type.priority;
                    if (T::Type.test(data)) {
                        ret = make_copyable_unique<Actor, T>(init, data);
                    }
                    else {
                        LOG_WARN("unable to instantiate object: {}:{}", T::Type.name.str, data.level_id.id);
                        ret = nullptr;
                    }
                    return ret;
                }
                else {
                    LOG_WARN("object not constructible with level data: {}:{}", T::Type.name.str, data.level_id.id);
                    return copyable_unique_ptr<Actor>{};
                }
            }
        });
	}

	static copyable_unique_ptr<Actor> createFromData(ActorInit init, ObjectLevelData& data);

	static const ObjectType* getType(size_t hash);
	static const ObjectType* getType(std::string_view name);
};

class Object : public Actor {
public:
	explicit Object(ActorInit init);
	Object(ActorInit init, ObjectLevelData& data);

    [[nodiscard]] const ObjectType*      object_type() const { return obj_type; };
    [[nodiscard]] const ObjectLevelData* object_data() const { return obj_data; };

private:
    ObjectLevelData* const  obj_data = nullptr;
    const ObjectType* const obj_type = nullptr;
};

template<std::derived_from<Object> T>
using ObjectTypeOf = void;



}
