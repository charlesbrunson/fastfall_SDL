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
#include "fastfall/game/object/objmessage.hpp"
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

struct ObjectType {

	struct Type {
		Type()
			: name()
			, hash(0lu)
		{
		}

		Type(const std::string& typeName)
			: name(typeName)
			, hash(std::hash<std::string>{}(typeName))
		{
		}

		std::string name;
		size_t	    hash;
	} type;

	bool allow_as_level_data = false;
	std::optional<AnimIDRef> anim;

	Vec2u tile_size = { 0u, 0u };
	Color tile_fill_color = Color::White().alpha(128u);
	Color tile_outline_color = Color::White;

    ActorPriority priority = ActorPriority::Normal;

	std::set<ObjectGroupTag> group_tags;
	std::set<ObjectProperty> properties;

	bool test(ObjectLevelData& data) const;
};

class Object;

template<typename T>
concept valid_object = std::derived_from<T, Object>;

struct ObjectFactory {
private:
	struct ObjectFactoryImpl {
		ObjectType object_type;
        copyable_unique_ptr<Actor>(*createfn)(ActorInit, ObjectLevelData& data);
	};

	static std::map<size_t, ObjectFactoryImpl>& getFactories();

public:
	template<typename T>
	requires valid_object<T>
	static void register_object(const ObjectType& type)
    {
        auto [it, inserted] = getFactories().emplace(
                type.type.hash,
                ObjectFactoryImpl{
                    .object_type    = type,
                });

        it->second.createfn =
        [](ActorInit init, ObjectLevelData& data) -> copyable_unique_ptr<Actor>
        {
            if constexpr (std::is_constructible_v<T, ActorInit, ObjectLevelData&>) {
                copyable_unique_ptr<Actor> ret;
                if (init.obj_type->test(data)) {
                    init.priority   = init.obj_type->priority;
                    ret = make_copyable_unique<Actor, T>(init, data);
                }
                else {
                    LOG_WARN("unable to instantiate object:{}:{}", init.obj_type->type.name, data.level_id.id);
                    ret = nullptr;
                }
                return ret;
            }
            else {
                LOG_WARN("object not constructible with level data:{}:{}", init.obj_type->type.name, data.level_id.id);
                return copyable_unique_ptr<Actor>{};
            }
        };
	}

	static copyable_unique_ptr<Actor> createFromData(ActorInit init, ObjectLevelData& data);

	static const ObjectType* getType(size_t hash);
	static const ObjectType* getType(std::string_view name);
};

class Object : public Actor {
public:
	explicit Object(ActorInit init);
	Object(ActorInit init, ObjectLevelData& data);
	~Object() override = default;


    [[nodiscard]] const ObjectType* object_type() const { return obj_type; };
    [[nodiscard]] const ObjectLevelData* object_data() const { return obj_data; };

protected:
	ObjectLevelData* const  obj_data = nullptr;

private:
    const ObjectType* const obj_type = nullptr;

    friend class ObjectFactory;
};

}
