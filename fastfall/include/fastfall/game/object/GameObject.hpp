#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/game/phys/Collidable.hpp"

#include <functional>
#include <type_traits>

#include "imgui.h"

#include "fastfall/game/InstanceID.hpp"
#include "fastfall/game/GameContext.hpp"

#include "fastfall/render/Drawable.hpp"

namespace ff {

struct GameObjectID {
	unsigned objID;
	size_t type;
};

//class GameObject;

class GameObject : public Drawable {
public:

	template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
	static constexpr void addType();

	static void build(GameContext instance, const ObjectRef& ref);
	static const std::string* lookupTypeName(size_t hash);

	GameObject(GameContext instance, const ObjectRef& ref) :
		context{ instance },
		id{ .objID = ref.id, .type = ref.type },
		objRef(&ref)
	{
		typeName = lookupTypeName(ref.type);
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