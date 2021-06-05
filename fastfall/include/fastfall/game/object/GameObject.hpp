#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/game/phys/Collidable.hpp"

#include <functional>
#include <type_traits>
#include <set>
#include <string>

#include "imgui.h"

#include "fastfall/game/InstanceID.hpp"
#include "fastfall/game/GameContext.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/util/log.hpp"

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
	explicit ObjectTypeProperty(std::string propName, std::string value_default) : name(propName), type(ObjectPropertyType::String), default_value(value_default) {}
	explicit ObjectTypeProperty(std::string propName, int value_default) : name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}
	explicit ObjectTypeProperty(std::string propName, bool value_default) : name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}
	explicit ObjectTypeProperty(std::string propName, float value_default) : name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}
	explicit ObjectTypeProperty(std::string propName, object_id value_default) : name(propName), type(ObjectPropertyType::String), default_value(std::to_string(value_default)) {}

	std::string name;
	ObjectPropertyType type;
	std::string default_value = "";

	inline bool operator< (const ObjectTypeProperty& rhs) const {
		return name < rhs.name;
	}
};

struct ObjectTypeConstraints {
	Vec2u tile_size = { 0u, 0u };
	std::set<ObjectTypeProperty> properties;

	bool test(ObjectRef& ref) const;
};


class GameObjectLibrary {
public:
	using ObjectTypeBuilder = std::function<std::unique_ptr<GameObject>(GameContext, const ObjectRef&, const ObjectTypeConstraints&)>;

	struct ObjectType {

		/*
		template<typename T>
		struct tag { using type = T; };
		*/

		template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
		static ObjectType create(std::string typeName, ObjectTypeConstraints&& constraints) {
			ObjectType t;
			//t.objTypeName = typeid(T).name();
			//t.objTypeName = t.objTypeName.substr(6); // cut off "class "
			t.objTypeName = typeName;

			t.hash = std::hash<std::string>{}(t.objTypeName);
			t.constraints = std::move(constraints);
			t.builder = [](GameContext inst, const ObjectRef& ref, const ObjectTypeConstraints& constraints)->std::unique_ptr<GameObject>
			{
				ObjectRef cref = ref;
				if (constraints.test(cref)) {
					return std::make_unique<T>(inst, ref);
				}
				else {
					LOG_WARN("unable to instantiate object:{}", ref.id);
					return nullptr;
				}
			};
			return t;
		};

		ObjectType() {
			hash = 0;
		}
		ObjectType(size_t typehash) {
			hash = typehash;
		}

		size_t hash;
		std::string objTypeName;
		ObjectTypeBuilder builder;
		ObjectTypeConstraints constraints;
	};

	struct ObjectType_compare {
		bool operator() (const ObjectType& lhs, const ObjectType& rhs) const {
			return lhs.hash < rhs.hash;
		}
	};

	static void build(GameContext instance, const ObjectRef& ref);

	static const std::string* lookupTypeName(size_t hash);

	template<typename T>
	struct Entry {
		Entry(std::string typeName) {
			GameObjectLibrary::addType<T>(typeName, ObjectTypeConstraints());
		}
		Entry(std::string typeName, ObjectTypeConstraints&& constraints) {
			GameObjectLibrary::addType<T>(typeName, std::move(constraints));
		}
	};


	static std::set<ObjectType, ObjectType_compare>& getBuilder() {
		if (!objectBuildMap) {
			objectBuildMap = std::make_unique<std::set<ObjectType, ObjectType_compare>>();
		}
		return *objectBuildMap;
	}

private:
	static std::unique_ptr<std::set<ObjectType, ObjectType_compare>> objectBuildMap;


	template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
	static void addType(std::string typeName, ObjectTypeConstraints&& constraints) {
		getBuilder().insert(std::move(ObjectType::create<T>(typeName, std::move(constraints))));
	}
};



class GameObject : public Drawable {
public:


	GameObject(GameContext instance, const ObjectRef& ref) :
		context{ instance },
		id{ .objID = ref.id, .type = ref.type },
		objRef(&ref)
	{
		typeName = GameObjectLibrary::lookupTypeName(ref.type);
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
	inline const std::string& getType()   const { return *typeName; };
	inline GameContext getContext()       const { return context; };
	inline int getDrawPriority() { return drawPriority; };
	inline const ObjectRef* getObjectRef() const { return objRef; };

	bool hasCollider = false;


protected:

	bool toDelete = false;

	virtual void draw(RenderTarget& target, RenderState states = RenderState()) const = 0;

	// drawPriority is used to determine drawing order between other game objects
	// [0, INT_MIN] = behind first FG tile layer, lowest is drawn first
	// [INT_MAX, 0) = behind first FG tile layer, lowest is drawn first
	int drawPriority = 1;

	GameContext context;

private:

	const ObjectRef* const objRef;
	const std::string* typeName;
	GameObjectID id;
};



}