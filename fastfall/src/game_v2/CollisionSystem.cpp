#include "fastfall/game_v2/CollisionSystem.hpp"

#include "fastfall/game_v2/phys/CollisionSolver.hpp"
#include "fastfall/game_v2/World.hpp"

#include <algorithm>
#include <execution>

#include "nlohmann/json.hpp"

namespace ff {

void CollisionSystem::update(secs deltaTime) {

	if (collision_dump)
	{
		(*collision_dump) = {
			{"frame", frame_count},
			{"delta", deltaTime}
		};
	}

	if (deltaTime > 0.0) 
	{
		size_t ndx = 0;
        for (auto& col : world->all_collidables()) {
            col.update(deltaTime);
        }
		for (auto& [id, arb] : arbiters)
		{
			colArb->gather_and_solve_collisions(deltaTime, regions, (collision_dump ? &(*collision_dump)["collisions"][ndx] : nullptr));
			ndx++;
		}
	}

	for (auto& colData : collidables) {
		colData->collidable.debug_draw();
	}
	collision_dump = nullptr;
	frame_count++;
};

void CollisionSystem::notify_created(ID<Collidable> id)
{
    arbiters[id] = CollidableArbiter{.collidable = id};
}

void CollisionSystem::notify_created(ID<ColliderRegion> id)
{
}

void CollisionSystem::notify_erased(ID<Collidable> id)
{
    arbiters.erase(id);
}

void CollisionSystem::notify_erased(ID<ColliderRegion> id)
{
    for (auto& [id, arb] : arbiters)
    {
        arb.erase_region(id);
    }
}

/*
Collidable* CollisionSystem::create_collidable(Vec2f init_pos, Vec2f init_size, Vec2f init_grav)
{
	collidables.push_back(std::make_unique<CollidableArbiter>(CollidableArbiter{
		.collidable = { init_pos, init_size, init_grav }, 
		.region_arbiters = {} 
		}));
	Collidable* col = &collidables.back()->collidable;
	return col;
}

bool CollisionSystem::erase_collidable(Collidable* collidable) {

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

bool CollisionSystem::erase_collider(ColliderRegion* region) {

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
*/

}
