#pragma once

#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include <functional>
#include <type_traits>
#include <set>
#include <string>
#include <typeindex>

#include "imgui.h"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/commandable.hpp"
#include "fastfall/game_v2/object/ObjectCommands.hpp"

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
	ObjectProperty(std::string propName, ObjectPropertyType type) : name(propName), type(type), default_value("") {}

	// specify type and default value
	ObjectProperty(std::string propName, std::string value_default): name(propName), type(ObjectPropertyType::String), default_value(value_default) {}
	ObjectProperty(std::string propName, int value_default)		: name(propName), type(ObjectPropertyType::Int),    default_value(std::to_string(value_default)) {}
	ObjectProperty(std::string propName, bool value_default)		: name(propName), type(ObjectPropertyType::Bool),   default_value(std::to_string(value_default)) {}
	ObjectProperty(std::string propName, float value_default)		: name(propName), type(ObjectPropertyType::Float),  default_value(std::to_string(value_default)) {}
	ObjectProperty(std::string propName, ObjLevelID value_default)	: name(propName), type(ObjectPropertyType::Object), default_value(std::to_string(value_default.id)) {}

	std::string name;
	ObjectPropertyType type;
	std::string default_value = "";

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

		Type(std::string typeName)
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

	std::set<ObjectGroupTag> group_tags;
	std::set<ObjectProperty> properties;

	bool test(ObjectLevelData& data) const;
};

class GameObject;

struct ObjSpawnID {
	static constexpr unsigned NO_ID = 0;

	unsigned id = NO_ID;

	bool operator== (const ObjSpawnID& rhs) const {
		return id == rhs.id;
	}
	bool operator< (const ObjSpawnID& rhs) const {
		return id < rhs.id;
	}

	operator bool() {
		return id != NO_ID;
	}
};

template<typename T>
concept valid_object =
std::is_base_of_v<GameObject, T>
&& std::is_same_v<decltype(T::Type), const ObjectType>
&& requires (T x)
{
	{ x.type() } -> std::same_as<const ObjectType&>;
};

struct ObjectFactory {
private:
	struct ObjectFactoryImpl {
		const ObjectType* object_type;
		std::unique_ptr<GameObject>(*createfn)(World* world, ObjectLevelData& data);
	};

	static std::map<size_t, ObjectFactoryImpl>& getFactories();

public:
	template<typename T>
	requires valid_object<T>
	static void register_object() {
		ObjectFactoryImpl factory;
		factory.object_type = &T::Type;
		factory.createfn = [](World* world, ObjectLevelData& data) -> std::unique_ptr<GameObject>
		{
			if constexpr (std::is_constructible_v<T, World*, ObjectLevelData&>) {
				std::unique_ptr<GameObject> ret;
				const ObjectType& type = T::Type;
				if (type.test(data)) {
					ret = std::make_unique<T>(world, data);
				}
				else {
					LOG_WARN("unable to instantiate object:{}:{}", type.type.name, data.level_id.id);
					ret = nullptr;
				}
				return ret;
			}
			else {
				const ObjectType& type = T::Type;
				LOG_WARN("object not constructible with level data:{}:{}", type.type.name, data.level_id.id);
				return nullptr;
			}
		};
		getFactories().insert(std::make_pair( factory.object_type->type.hash, std::move(factory) ));
	}


	template<typename T, typename ... Args>
	requires valid_object<T> && std::is_constructible_v<T, World*, Args...>
	static std::unique_ptr<GameObject>&& create(World* world, Args&&... args)
	{
		std::unique_ptr<GameObject> obj = std::make_unique<T>(world, args...);
		if (obj) {
			return std::move(obj);
		}
		else {
			LOG_ERR_("Failed to create object: {}", T::Type.type.name);
		}
		return std::move(std::unique_ptr<GameObject>{});
	}

	static std::unique_ptr<GameObject>&& createFromData(World* world, ObjectLevelData& data);

	static const ObjectType* getType(size_t hash);

	static const ObjectType* getType(std::string_view name);

};

class GameObject : public Commandable<ObjCmd> {
public:
	GameObject(World* w);
	GameObject(World* w, ObjectLevelData& data);
	virtual ~GameObject() = default;

	virtual void update(secs deltaTime) = 0;
	virtual void predraw(float interp, bool updated) = 0;

    virtual void set_world(World* w) { world = w; }

	virtual const ObjectType& type() const = 0;

	virtual void ImGui_Inspect() {
		ImGui::Text("Hello World!");
	};

	inline const ObjectLevelData* level_data() const { return m_data; };

	bool m_has_collider = false;
	bool m_show_inspect = false;

protected:
	ObjectLevelData* const m_data = nullptr;
    World* world;
};



}