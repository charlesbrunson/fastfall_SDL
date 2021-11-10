#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include <functional>
#include <type_traits>
#include <set>
#include <string>

#include "imgui.h"

//#include "fastfall/game/InstanceID.hpp"
#include "fastfall/game/GameContext.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/commandable.hpp"
#include "fastfall/game/object/ObjectCommands.hpp"

namespace ff {

struct GameObjectID {
	unsigned spawnID;
	std::optional<unsigned> levelID;
};

//class GameObject;

enum class ObjectPropertyType {
	String,
	Int,
	Bool,
	Float,
	Object,
};

struct ObjectTypeProperty {

	// only specify the expected type
	explicit ObjectTypeProperty(std::string propName, ObjectPropertyType type) : name(propName), type(type), default_value("") {}

	// specify type and default value
	explicit ObjectTypeProperty(std::string propName, std::string value_default): name(propName), type(ObjectPropertyType::String), default_value(value_default) {}
	explicit ObjectTypeProperty(std::string propName, int value_default)		: name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}
	explicit ObjectTypeProperty(std::string propName, bool value_default)		: name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}
	explicit ObjectTypeProperty(std::string propName, float value_default)		: name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}
	explicit ObjectTypeProperty(std::string propName, object_id value_default)	: name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}

	std::string name;
	ObjectPropertyType type;
	std::string default_value = "";

	inline bool operator< (const ObjectTypeProperty& rhs) const {
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

	std::optional<AnimIDRef> anim;
	Vec2u tile_size = { 0u, 0u };
	std::set<ObjectGroupTag> group_tags;
	std::set<ObjectTypeProperty> properties;

	bool test(ObjectData& data) const;
};

class GameObject;

class GameObjectLibrary {
private:

	using ObjectTypeBuilderFn = std::function<std::unique_ptr<GameObject>(GameContext, const ObjectType&, const ObjectData&, std::optional<unsigned>)>;

	struct ObjectTypeBuilder {

		template<typename T>
			requires std::is_base_of_v<GameObject, T> 
				&& std::is_constructible_v<T, GameContext, const ObjectType&, const ObjectData&, std::optional<unsigned>>
		static ObjectTypeBuilder create(std::string typeName, ObjectType&& constraints) {
			ObjectTypeBuilder t;
			t.objTypeName = typeName;
			t.hash = std::hash<std::string>{}(t.objTypeName);
			t.constraints = std::move(constraints);
			t.builder = [](
				GameContext instance, 
				const ObjectType& objtype, 
				const ObjectData& objdata, 
				std::optional<unsigned> levelID
				) 
				-> std::unique_ptr<GameObject>
			{
				ObjectData data = objdata;
				if (objtype.test(data)) {
					return std::make_unique<T>(instance, objtype, data, levelID);
				}
				else {
					LOG_WARN("unable to instantiate object:{}:{}", objtype.type.name, (levelID ? std::to_string(levelID.value()) : "?"));
					return nullptr;
				}
			};
			return t;
		};

		ObjectTypeBuilder() {
			hash = 0;
		}
		ObjectTypeBuilder(size_t typehash) {
			hash = typehash;
		}

		size_t hash;
		std::string objTypeName;
		ObjectTypeBuilderFn builder;
		ObjectType constraints;
	};

	struct ObjectTypeBuilder_compare {
		bool operator() (const ObjectTypeBuilder& lhs, const ObjectTypeBuilder& rhs) const {
			return lhs.hash < rhs.hash;
		}
	};

	static std::set<ObjectTypeBuilder, ObjectTypeBuilder_compare>& getBuilder() {
		static std::set<ObjectTypeBuilder, ObjectTypeBuilder_compare> objectBuildMap;
		return objectBuildMap;
	}

public:

	static void build(GameContext instance, const ObjectData& data, std::optional<unsigned> levelID = std::nullopt);

	static const std::string* lookupTypeName(size_t hash);

	template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
	static void addType(ObjectType&& constraints) {
		getBuilder().insert(std::move(ObjectTypeBuilder::create<T>(constraints.type.name, std::move(constraints))));
	}

};

class GameObject : public Commandable<ObjCmd> {
public:

	GameObject(GameContext instance, const ObjectType& objtype, const ObjectData& objdata, std::optional<unsigned> levelID);

	virtual ~GameObject() = default;
	virtual std::unique_ptr<GameObject> clone() const = 0;
	virtual void update(secs deltaTime) = 0;
	virtual void predraw(secs deltaTime) = 0;

	bool showInspect = false;
	virtual void ImGui_Inspect() {
		ImGui::Text("Hello World!");
	};

	inline GameObjectID getID()             const { return m_id; };
	inline const std::string& getTypeName() const { return m_type.get().type.name; };
	inline const size_t getTypeHash()		const { return m_type.get().type.hash; };

	inline const ObjectType& getObjType()   const { return m_type; };

	inline const ObjectData& getObjData() const { return m_data; };

	inline GameContext getContext()         const { return context; };

	bool hasCollider = false;

protected:

	bool toDelete = false;

	GameContext context;

private:
	const ObjectData m_data;
	const std::reference_wrapper<const ObjectType> m_type;
	GameObjectID m_id;
};

}