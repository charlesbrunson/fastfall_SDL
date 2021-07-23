#pragma once

#include "InstanceID.hpp"

#include "fastfall/game/phys/Identity.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"

#include <optional>
#include <memory>
#include <map>
#include <string>


//class GameInstance;

// lightweight containing interfaces to all the game instance's managers
// meant to be passed to lower game entities and the like

namespace ff {

class GameObject;
class GameObjectManager;
class CollisionManager;
class TriggerManager;
class GameCamera;
class Level;

class ColliderRegion;
class ColliderQuad;

class ObjectContext {
private:
	ObjectContext(InstanceID instanceID);

	friend class GameContext;
public:
	InstanceID id;

	// add more as needed
	void add(std::unique_ptr<GameObject>&& obj);
	GameObject* getByID(unsigned int objectID);

	// summons the full manager, if necessary
	GameObjectManager& get() const;
	GameObjectManager* operator-> ();

};

class CollisionContext {
private:
	CollisionContext(InstanceID instanceID);

	friend class GameContext;
public:
	InstanceID id;

	// summons the full manager, if necessary
	CollisionManager& get() const;
	CollisionManager* operator-> ();

	// add more as needed
	const ColliderRegion* get_region(ColliderID collider_id) const noexcept;
	const ColliderRegion* get_region(const PersistantContact& contact) const noexcept;
	const ColliderQuad* get_quad(ColliderID collider_id, int quad_id) const noexcept;
	const ColliderQuad* get_quad(const PersistantContact& contact) const noexcept;
};

class CameraContext {
private:
	CameraContext(InstanceID instanceID);

	friend class GameContext;
public:
	InstanceID id;

	// add more as needed

	// summons the full manager, if necessary
	GameCamera& get() const;
	GameCamera* operator-> ();
};

class LevelContext {
private:
	LevelContext(InstanceID instanceID);

	friend class GameContext;
public:
	InstanceID id;

	// summons the active level, if there is one
	std::optional<std::reference_wrapper<Level>> get_active() const;
	std::map<const std::string*, std::unique_ptr<Level>>& get_all() const;

	// add more as needed
};

class TriggerContext {
private:
	TriggerContext(InstanceID instanceID);

	friend class GameContext;
public:
	InstanceID id;

	// add more as needed
	TriggerManager& get() const;
	TriggerManager* operator-> ();
};


class GameContext {
private:

	GameContext();
	GameContext(InstanceID instanceID);

	friend class GameInstance;
	friend class InstanceObserver;

	InstanceID id;

public:
	GameContext(const GameContext& context) = default;
	GameContext& operator=(const GameContext& context) = default;

	inline InstanceID getID() const noexcept { return id; };
	bool valid() const noexcept;

	inline ObjectContext	objects()	noexcept { return ObjectContext{ id };		};
	inline CollisionContext collision() noexcept { return CollisionContext{ id };	};
	inline CameraContext	camera()	noexcept { return CameraContext{ id };		};
	inline LevelContext		levels()	noexcept { return LevelContext{	id };		};
	inline TriggerContext	triggers()	noexcept { return TriggerContext{ id };		};
};

}