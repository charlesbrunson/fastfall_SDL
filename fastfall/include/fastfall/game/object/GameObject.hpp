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

struct ObjectConfig {
	const GameContext context;
	const std::reference_wrapper<const ObjectType> m_type;
	const ObjectLevelData* const m_level_data = nullptr;
};




class GameObjectLibrary {
private:

	using ObjectTypeBuilderFn = std::function<std::unique_ptr<GameObject>(GameContext, const ObjectType&, ObjectLevelData&)>;

	struct ObjectTypeBuilder {
		ObjectTypeBuilder(size_t _hash, std::string _typename, std::type_index _type_ndx, ObjectType&& _type, ObjectTypeBuilderFn&& _builder)
			: hash(_hash)
			, objTypeName(_typename)
			, type_ndx(_type_ndx)
			, constraints(std::move(_type))
			, builder(std::move(_builder))
		{

		}
		size_t hash;
		std::string objTypeName;
		std::type_index type_ndx;
		ObjectType constraints;
		ObjectTypeBuilderFn builder;
	};

	struct ObjectTypeBuilder_compare {
		bool operator() (const ObjectTypeBuilder& lhs, const ObjectTypeBuilder& rhs) const {
			return lhs.hash < rhs.hash;
		}
	};


	template<typename T>
	requires std::is_base_of_v<GameObject, T>
	static ObjectTypeBuilder create(std::string typeName, ObjectType&& constraints)
	{
		return ObjectTypeBuilder(
			std::hash<std::string>{}(typeName),
			typeName,
			typeid(T),
			std::move(constraints),
			[](GameContext context, const ObjectType& type, ObjectLevelData& data) -> std::unique_ptr<GameObject> {
				if constexpr (std::is_constructible_v<T, ObjectConfig>) {
					if (type.test(data)) {

						ObjectConfig cfg{
							.context = context,
							.m_type = type,
							.m_level_data = &data
						};
						return std::make_unique<T>(cfg);
					}
					else {
						LOG_WARN("unable to instantiate object:{}:{}", type.type.name, data.level_id.id);
						return nullptr;
					}
				}
				return nullptr;
			}
		);
	}

	static std::set<ObjectTypeBuilder, ObjectTypeBuilder_compare>& getBuilder() {
		static std::set<ObjectTypeBuilder, ObjectTypeBuilder_compare> objectBuildMap;
		return objectBuildMap;
	}


public:

	template<typename T>
	requires std::is_base_of_v<GameObject, T>
	static const ObjectType* getType() {
		auto& builder = getBuilder();

		for (auto& type_builder : builder) {
			if (type_builder.type_ndx == typeid(T)) {
				return &type_builder.constraints;
			}
		}
		return nullptr;
	}

	static GameObject* buildFromData(GameContext instance, ObjectLevelData& data);

	static const std::string* lookupTypeName(size_t hash);

	template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
	static void addType(ObjectType&& constraints) {
		getBuilder().insert(std::move(create<T>(constraints.type.name, std::move(constraints))));
	}

};


class GameObject : public Commandable<ObjCmd> {
public:
	GameObject(ObjectConfig cfg);

	virtual ~GameObject() = default;
	virtual std::unique_ptr<GameObject> clone() const = 0;
	virtual void update(secs deltaTime) = 0;
	virtual void predraw(secs deltaTime) = 0;

	bool showInspect = false;

	virtual void ImGui_Inspect() {
		ImGui::Text("Hello World!");
	};

	const ObjectConfig& getConfig() const { return m_config; }
	inline GameContext getContext() const { return m_config.context; }
	inline const ObjectType& getType() const { return m_config.m_type; }
	inline const ObjectLevelData* getLevelData() const { return m_config.m_level_data; }
	inline ObjSpawnID getID() const { return m_spawnID; };

	inline bool can_delete() const { return toDelete; };

	bool hasCollider = false;

protected:
	bool toDelete = false;

private:
	const ObjSpawnID m_spawnID;
	const ObjectConfig m_config;
};

}