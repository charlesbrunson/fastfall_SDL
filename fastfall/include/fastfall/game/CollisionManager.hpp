#pragma once

//#include "util/Updatable.hpp"

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/game/phys/Collidable.hpp"
//#include "phys/CollisionMap.hpp"
//#include "phys/Arbiter.hpp"

#include "fastfall/game/phys/RegionArbiter.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"

#include "fastfall/game/phys/CollisionSolver.hpp"
//#include "phys/Raycast.hpp"

#include <vector>
#include <list>
#include <memory>

namespace ff {

class CollisionManager /* : public sf::Drawable*/ {
public:
	constexpr static unsigned MAX_COLLIDABLES = 128u;


	using Collidable_Wptr = std::weak_ptr<Collidable>;
	using ColliderRegion_Wptr = std::weak_ptr<ColliderRegion>;
	using RegionArbiterMap = std::map<ColliderRegion_Wptr, RegionArbiter, std::owner_less<ColliderRegion_Wptr>>;

	struct ArbiterData {
		ArbiterData(Collidable* _col) :
			collidable(_col)
		{

		}

		//CollisionFrame frame;
		RegionArbiterMap regions;
		Collidable* collidable;
	};

	using ArbiterMap = std::map<const Collidable*, ArbiterData>;

	struct Colliders {
		std::vector<ColliderRegion_Wptr> colliders_free;         // static collider
		std::vector<ColliderRegion_Wptr> colliders_attached;     // collider attached to a collidable
	};
	struct Collidables {
		std::list<Collidable> collidables_attached; // collidable with colliders attached
		std::list<Collidable> collidables_free;     // free collidables
	};

	CollisionManager(unsigned instance);

	void update(secs deltaTime);
	//void predraw(secs deltaTime);

	void addColliderRegion(std::shared_ptr<ColliderRegion> col);

	//void addCollidable(std::shared_ptr<Collidable> col);
	Collidable* createCollidable(Collidable&& col);
	void removeCollidable(Collidable* colptr);

	inline const Colliders* getColliders() const { return &colliders; };
	inline const Collidables* getCollidables() const { return &collidables; };
	inline const ArbiterMap* getArbiters() const { return &arbiters; };

private:

	void cleanInvalidCollisionObjects();

	void broadPhase(std::vector<ColliderRegion_Wptr>& p_colliders, std::list<Collidable>& p_collidables, secs deltaTime);

	void solve(ArbiterData& arbData);

	unsigned instanceID;

	std::vector<std::pair<Contact, Vec2f>> contacts;

	Colliders colliders;
	Collidables collidables;

	ArbiterMap arbiters;
};

}