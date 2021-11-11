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

struct ObjectProperty {

	// only specify the expected type
	explicit ObjectProperty(std::string propName, ObjectPropertyType type) : name(propName), type(type), default_value("") {}

	// specify type and default value
	explicit ObjectProperty(std::string propName, std::string value_default): name(propName), type(ObjectPropertyType::String), default_value(value_default) {}
	explicit ObjectProperty(std::string propName, int value_default)		: name(propName), type(ObjectPropertyType::Int),    default_value(std::to_string(value_default)) {}
	explicit ObjectProperty(std::string propName, bool value_default)		: name(propName), type(ObjectPropertyType::Bool),   default_value(std::to_string(value_default)) {}
	explicit ObjectProperty(std::string propName, float value_default)		: name(propName), type(ObjectPropertyType::Float),  default_value(std::to_string(value_default)) {}
	explicit ObjectProperty(std::string propName, object_id value_default)	: name(propName), type(ObjectPropertyType::Object), default_value(std::to_string(value_default)) {}

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

	std::optional<AnimIDRef> anim;
	Vec2u tile_size = { 0u, 0u };
	std::set<ObjectGroupTag> group_tags;
	std::set<ObjectProperty> properties;

	bool test(ObjectData& data) const;
};

class GameObject;

struct ObjectTemplate {

	ObjectTemplate(const ObjectType& type, ObjectData& data, unsigned level_id = 0u) 
		: m_type(type)
		, m_data(data)
		, m_levelID(level_id)
	{
		m_valid = m_type.get().test(m_data.get());
	}

	inline bool valid()				const { return m_valid; };
	inline unsigned level_id()	const { return m_levelID; };
	inline const ObjectType& type() const { return m_type.get(); };
	inline const ObjectData& data() const { return m_data.get(); };

private:
	bool m_valid;
	unsigned m_levelID;
	const std::reference_wrapper<const ObjectType> m_type;
	const std::reference_wrapper<ObjectData> m_data;
};

class GameObjectLibrary {
private:

	using ObjectTypeBuilderFn = std::function<std::unique_ptr<GameObject>(GameContext, ObjectTemplate)>;

	struct ObjectTypeBuilder {

		template<typename T>
			requires std::is_base_of_v<GameObject, T> 
				&& std::is_constructible_v<T, GameContext, ObjectTemplate>
		static ObjectTypeBuilder create(std::string typeName, ObjectType&& constraints) {
			ObjectTypeBuilder t;
			t.objTypeName = typeName;
			t.hash = std::hash<std::string>{}(t.objTypeName);
			t.constraints = std::move(constraints);
			t.builder = [](
				GameContext instance, ObjectTemplate obj_template
				) 
				-> std::unique_ptr<GameObject>
			{
				if (obj_template.valid()) {
					return std::make_unique<T>(instance, obj_template);
				}
				else {
					LOG_WARN("unable to instantiate object:{}:{}", obj_template.type().type.name, obj_template.level_id());
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

	static GameObject* buildFromLevel(GameContext instance, ObjectData& data, unsigned levelID);

	static const std::string* lookupTypeName(size_t hash);

	template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
	static void addType(ObjectType&& constraints) {
		getBuilder().insert(std::move(ObjectTypeBuilder::create<T>(constraints.type.name, std::move(constraints))));
	}

};


class GameObject : public Commandable<ObjCmd> {
public:

	GameObject(GameContext instance, std::optional<ObjectTemplate> templ_data);

	virtual ~GameObject() = default;
	virtual std::unique_ptr<GameObject> clone() const = 0;
	virtual void update(secs deltaTime) = 0;
	virtual void predraw(secs deltaTime) = 0;

	bool showInspect = false;

	virtual void ImGui_Inspect() {
		ImGui::Text("Hello World!");
	};

	inline const ObjectTemplate* getTemplate() const { 
		return template_data ? &template_data.value() : nullptr; 
	};

	inline GameContext getContext() const { 
		return context; 
	};

	inline unsigned getID() const { return spawnID; };

	inline bool can_delete() const { return toDelete; };

	bool hasCollider = false;

protected:

	bool toDelete = false;

	GameContext context;

private:
	unsigned spawnID;
	std::optional<ObjectTemplate> template_data;
};

}