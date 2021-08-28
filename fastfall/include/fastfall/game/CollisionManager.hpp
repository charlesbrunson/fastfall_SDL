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

#include "ext/plf_colony.h"

#include <vector>
#include <list>
#include <memory>

namespace ff {

class CollisionManager {
public:
	struct CollidableData {
		CollidableData(Collidable&& col) :
			collidable(col)
		{

		}


		Collidable collidable;
		std::vector<RegionArbiter> regionArbiters;
	};

private:
	plf::colony<CollidableData> collidables;
	std::vector<std::unique_ptr<ColliderRegion>> regions;

public:



	CollisionManager(unsigned instance);

	void update(secs deltaTime);
	
	Collidable* create_collidable();
	Collidable* create_collidable(Vec2f init_pos, Vec2f init_size, Vec2f init_grav = Vec2f{});
	bool erase_collidable(Collidable* collidable);
	
	template<ColliderType T, typename ... Args>
	T* create_collider(Args&&... args) {
		std::unique_ptr<ColliderRegion> collider = std::make_unique<T>(args...);
		T* collider_ptr = static_cast<T*>(collider.get());
		regions.push_back(std::move(collider));
		return collider_ptr;
	}
	bool erase_collider(ColliderRegion* region);

	inline const std::vector<std::unique_ptr<ColliderRegion>>& get_colliders() const { return regions; };
	inline const plf::colony<CollidableData>& get_collidables() const { return collidables; };

private:

	void broadPhase(secs deltaTime);

	void updateRegionArbiters(CollidableData& data);

	void updatePushBound(Rectf& push_bound, const std::array<float, 4u>& boundDist, const Contact* contact);

	void solve(CollidableData& collidableData);

	unsigned instanceID;
	

};

}