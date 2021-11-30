#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include <functional>
#include <type_traits>
#include <set>
#include <string>
#include <typeindex>

#include "imgui.h"

//#include "fastfall/game/InstanceID.hpp"
#include "fastfall/game/GameContext.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/commandable.hpp"
#include "fastfall/game/object/ObjectCommands.hpp"

namespace ff {


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
	explicit ObjectProperty(std::string propName, ObjectPropertyType type) : name(propName), type(type), default_value("") {}

	// specify type and default value
	explicit ObjectProperty(std::string propName, std::string value_default): name(propName), type(ObjectPropertyType::String), default_value(value_default) {}
	explicit ObjectProperty(std::string propName, int value_default)		: name(propName), type(ObjectPropertyType::Int),    default_value(std::to_string(value_default)) {}
	explicit ObjectProperty(std::string propName, bool value_default)		: name(propName), type(ObjectPropertyType::Bool),   default_value(std::to_string(value_default)) {}
	explicit ObjectProperty(std::string propName, float value_default)		: name(propName), type(ObjectPropertyType::Float),  default_value(std::to_string(value_default)) {}
	explicit ObjectProperty(std::string propName, ObjLevelID value_default)	: name(propName), type(ObjectPropertyType::Object), default_value(std::to_string(value_default.id)) {}

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

	bool allow_level_data = false;
	std::optional<AnimIDRef> anim;
	Vec2u tile_size = { 0u, 0u };
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
	//{ T::Type  } -> std::same_as<const ObjectType>;
	{ x.type() } -> std::same_as<const ObjectType&>;
	//&T::Type == &x.type();
};

struct ObjectFactory {
private:
	struct ObjectFactoryImpl {
		const ObjectType* object_type;
		std::unique_ptr<GameObject>(*createfn)(GameContext cfg, ObjectLevelData& data);
	};

	static std::map<size_t, ObjectFactoryImpl>& getFactories();

	static GameObject* add_obj_to_instance(GameContext cfg, std::unique_ptr<GameObject>&& obj);

public:
	template<typename T>
	requires valid_object<T>
	static void register_object() {
		ObjectFactoryImpl factory;
		factory.object_type = &T::Type;
		factory.createfn = [](GameContext cfg, ObjectLevelData& data) -> std::unique_ptr<GameObject> 
		{
			if constexpr (std::is_constructible_v<T, GameContext, ObjectLevelData&>) {
				std::unique_ptr<GameObject> ret;
				const ObjectType& type = T::Type;
				if (type.test(data)) {
					ret = std::make_unique<T>(cfg, data);
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
	requires valid_object<T> && std::is_constructible_v<T, GameContext, Args...>
	static GameObject* create(GameContext cfg, Args&&... args) 
	{
		std::unique_ptr<GameObject> obj = std::make_unique<T>(cfg, args...);
		if (obj) {
			return add_obj_to_instance(cfg, std::move(obj));
		}
		else {
			LOG_ERR_("Failed to create object: {}", T::Type.type.name);
		}
		return nullptr;
	}

	static GameObject* createFromData(GameContext cfg, ObjectLevelData& data);

	static const ObjectType* getType(size_t hash);

	static const ObjectType* getType(std::string_view name);

};

class GameObject : public Commandable<ObjCmd> {
public:
	GameObject(GameContext cfg);
	GameObject(GameContext cfg, ObjectLevelData& data);

	virtual ~GameObject() = default;

	virtual std::unique_ptr<GameObject> clone() const = 0;
	virtual void update(secs deltaTime) = 0;
	virtual void predraw(secs deltaTime) = 0;

	virtual const ObjectType& type() const = 0;

	virtual void ImGui_Inspect() {
		ImGui::Text("Hello World!");
	};

	inline GameContext context() const { return m_context; };
	inline ObjSpawnID spawn_id() const { return m_spawnID; };
	inline const ObjectLevelData* level_data() const { return m_data; };
	inline bool can_remove() const { return m_remove; };

	bool m_has_collider = false;
	bool m_show_inspect = false;

protected:
	bool m_remove = false;

	const GameContext m_context;
	const ObjSpawnID m_spawnID;
	ObjectLevelData* const m_data = nullptr;
};



}