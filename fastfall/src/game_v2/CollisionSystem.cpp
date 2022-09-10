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

    auto& collidables = world->all<Collidable>();
    auto& colliders = world->all<ColliderRegion>();

	if (deltaTime > 0.0) 
	{
		size_t ndx = 0;
        for (auto& col : collidables) {
            col.update(&colliders, deltaTime);
        }

		for (auto& [id, arb] : arbiters)
		{
            auto* dump_ptr = (collision_dump ? &(*collision_dump)["collisions"][ndx] : nullptr);
			arb.gather_and_solve_collisions(world->at(id), colliders, deltaTime, dump_ptr);
			ndx++;
		}
	}

	for (auto& collidable : collidables) {
		collidable.debug_draw();
	}
	collision_dump = nullptr;
	frame_count++;
};

void CollisionSystem::set_world(World* w) {
    world = w;
    // update all collidable & surface tracker pointers
    for (auto& collidable : w->all<Collidable>()) {
        for (auto& [id, tracker] : collidable.tracker_ids) {
            tracker = world->get(id);
            tracker->set_collidable_ptr(&collidable);
        }
    }
}

void CollisionSystem::notify_created(ID<Collidable> id)
{
    arbiters[id] = CollidableArbiter{.collidable_id = id};
}

void CollisionSystem::notify_created(ID<ColliderRegion> id)
{
}

void CollisionSystem::notify_created(ID<SurfaceTracker> id)
{
    auto& tracker = world->at(id);
    auto& collidable = world->at(tracker.get_collidable_id());
    collidable.tracker_ids.emplace_back(std::pair<ID<SurfaceTracker>, SurfaceTracker*>{ id, &tracker });
}

void CollisionSystem::notify_erased(ID<Collidable> id)
{
    arbiters.erase(id);
}

void CollisionSystem::notify_erased(ID<ColliderRegion> id)
{
    for (auto& [_, arb] : arbiters)
    {
        arb.erase_region(id);
    }
}

void CollisionSystem::notify_erased(ID<SurfaceTracker> id)
{
    auto& tracker = world->at(id);
    auto& collidable = world->at(tracker.get_collidable_id());
    std::erase_if(collidable.tracker_ids, [id](const auto& pair) { return pair.first == id; });
}

}
