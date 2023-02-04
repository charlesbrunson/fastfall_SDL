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

    inline bool operator< (const ObjectProperty& rhs) const {
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

struct ObjectInit : public ActorInit {
    const ObjectType* object_type;
};

class Object;

template<typename T>
concept valid_object = std::derived_from<T, Object>;

struct ObjectFactory {
private:

    struct ObjectBuilder {
        ObjectType type;
        std::function<copyable_unique_ptr<Actor>(ActorInit, ObjectLevelData&)> create;
    };

	static std::unordered_map<size_t, ObjectBuilder> object_builders;
    static std::unordered_map<std::type_index, size_t> type_to_hash;

public:
	template<valid_object T, class... Args>
	static void register_object(ObjectType type, Args&&... args)
    {
        assert(!object_builders.contains(type.name.hash));

        auto [it, _] = object_builders.emplace(type.name.hash, ObjectBuilder{ .type = type });
        auto& type_ref = it->second.type;
        it->second.create = [&type_ref, args...](ActorInit init, ObjectLevelData& data) -> copyable_unique_ptr<Actor>
        {
            if constexpr (std::is_constructible_v<T, ObjectInit, ObjectLevelData&, Args...>) {
                ObjectInit obj_init { init };
                obj_init.priority = type_ref.priority;
                obj_init.object_type = &type_ref;

                copyable_unique_ptr<Actor> ret;
                if (type_ref.test(data)) {
                    ret = make_copyable_unique<Actor, T>(obj_init, data, std::forward<Args>(args)...);
                }
                else {
                    LOG_WARN("unable to instantiate object: {}:{}", type_ref.name.str, data.level_id.id);
                    ret = nullptr;
                }
                return ret;
            }
            else {
                LOG_WARN("object not constructible with level data: {}:{}", type_ref.name.str, data.level_id.id);
                return copyable_unique_ptr<Actor>{};
            }
        };

        type_to_hash.emplace(typeid(T), type.name.hash);
	}

	static copyable_unique_ptr<Actor> createFromData(ActorInit init, ObjectLevelData& data);
	static const ObjectType* getType(size_t hash);
	static const ObjectType* getType(std::string_view name);

    template<class T>
    static const ObjectType* getType() {
        return type_to_hash.contains(typeid(T))
            ? getType(type_to_hash.at(typeid(T)))
            : nullptr;
    }
};

class Object : public Actor {
public:
	explicit Object(ObjectInit init);
	Object(ObjectInit init, ObjectLevelData& data);

    [[nodiscard]] const ObjectType*      object_type() const { return obj_type; };
    [[nodiscard]] const ObjectLevelData* object_data() const { return obj_data; };

private:
    const ObjectLevelData* const obj_data = nullptr;
    const ObjectType*      const obj_type = nullptr;
};

}
