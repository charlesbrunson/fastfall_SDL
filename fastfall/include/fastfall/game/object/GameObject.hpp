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
	unsigned objID;
	size_t type;
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
	std::string typeName;
	std::optional<AnimIDRef> anim;
	Vec2u tile_size = { 0u, 0u };
	std::set<ObjectGroupTag> group_tags;
	std::set<ObjectTypeProperty> properties;

	bool test(ObjectData& ref) const;
};

class GameObject;

class GameObjectLibrary {
private:

	using ObjectTypeBuilderFn = std::function<std::unique_ptr<GameObject>(GameContext, const ObjectData&, const ObjectType&)>;

	struct ObjectTypeBuilder {

		template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
		static ObjectTypeBuilder create(std::string typeName, ObjectType&& constraints) {
			ObjectTypeBuilder t;
			t.objTypeName = typeName;

			t.hash = std::hash<std::string>{}(t.objTypeName);
			t.constraints = std::move(constraints);
			t.builder = [](GameContext inst, const ObjectData& ref, const ObjectType& constraints) -> std::unique_ptr<GameObject>
			{
				ObjectData cref = ref;
				if (constraints.test(cref)) {
					return std::make_unique<T>(inst, ref, constraints);
				}
				else {
					LOG_WARN("unable to instantiate object:{}", ref.id);
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

	static void build(GameContext instance, const ObjectData& ref);

	static const std::string* lookupTypeName(size_t hash);

	template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
	static void addType(ObjectType&& constraints) {
		getBuilder().insert(std::move(ObjectTypeBuilder::create<T>(constraints.typeName, std::move(constraints))));
	}

};


class GameObject : public Commandable<ObjCmd> {
public:

	GameObject(GameContext instance, const ObjectData& ref, const ObjectType& objtype) :
		context{ instance },
		id{ .objID = ref.id, .type = ref.typehash },
		objRef(&ref),
		type(&objtype)
	{
	};
	virtual ~GameObject() = default;
	virtual std::unique_ptr<GameObject> clone() const = 0;
	virtual void update(secs deltaTime) = 0;
	virtual void predraw(secs deltaTime) = 0;

	bool showInspect = false;
	virtual void ImGui_Inspect() {
		ImGui::Text("Hello World!");
	};

	inline unsigned getID()               const { return id.objID; };
	inline const std::string& getTypeName()   const { return type->typeName; };
	inline const ObjectType& getType()   const { return *type; };
	inline const ObjectData& getObjectRef() const { return *objRef; };

	inline GameContext getContext()       const { return context; };

	bool hasCollider = false;

protected:

	bool toDelete = false;

	GameContext context;

private:
	const ObjectData* const objRef;
	const ObjectType* const type;
	GameObjectID id;
};



}