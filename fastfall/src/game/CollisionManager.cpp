#include "fastfall/game/CollisionManager.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

#include <algorithm>
#include <execution>
#include <set>

#include "fastfall/game/phys/CollisionSolver.hpp"
#include "fastfall/game/Instance.hpp"

#include "nlohmann/json.hpp"

namespace ff {

CollisionManager::CollisionManager(unsigned instance) :
	instanceID(instance)
{

}

void CollisionManager::update(secs deltaTime) {

	if (collision_dump)
	{
		(*collision_dump) = {
			{"frame", frame_count},
			{"delta", deltaTime}
		};
	}

	if (deltaTime > 0.0) {
		for (auto& colArb : collidables) {
			colArb->gather_and_solve_collisions(deltaTime, regions, collision_dump);
		}
	}
	for (auto& colData : collidables) {
		colData->collidable.debug_draw();
	}
	collision_dump = nullptr;
	frame_count++;
	frame_collision_count = 0u;
};

// --------------------------------------------------------------

Collidable* CollisionManager::create_collidable() {
	collidables.push_back(std::make_unique<CollidableArbiter>());
	return &collidables.back()->collidable;
}
Collidable* CollisionManager::create_collidable(Vec2f init_pos, Vec2f init_size, Vec2f init_grav) {
	Collidable* col = create_collidable();
	col->init(init_pos, init_size, init_grav);
	return col;
}

bool CollisionManager::erase_collidable(Collidable* collidable) {

	auto it = std::find_if(collidables.begin(), collidables.end(), 
		[&collidable](const auto& cArb) {
			return collidable == &cArb->collidable;
		});

	if (it != collidables.end()) {
		collidables.erase(it);
		return true;
	}
	return false;
}

bool CollisionManager::erase_collider(ColliderRegion* region) {

	for (auto& colArb : collidables) {
		colArb->erase_region(region);
	}

	auto it = std::find_if(regions.begin(), regions.end(),
		[&region](const auto& rptr) {
			return rptr.get() == region;
		});

	if (it != regions.end()) {
		regions.erase(it);
		return true;
	}
	return false;
}


}
