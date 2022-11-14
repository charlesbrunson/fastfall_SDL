#include "fastfall/game/CollisionSystem.hpp"

#include "fastfall/game/phys/CollisionSolver.hpp"
#include "fastfall/game/World.hpp"

#include <algorithm>
#include <execution>

#include "nlohmann/json.hpp"

namespace ff {

void CollisionSystem::update(World& world, secs deltaTime)
{
	if (collision_dump)
	{
		(*collision_dump) = {
			{"frame", frame_count},
			{"delta", deltaTime}
		};
	}

    auto& collidables = world.all<Collidable>();
    auto& colliders = world.all<ColliderRegion>();

	if (deltaTime > 0.0) 
	{
        for(auto& [id, collider] : colliders) {
            collider->update(deltaTime);
        }

		size_t ndx = 0;
        for (auto [id, col] : collidables) {
            col.update(&colliders, deltaTime);
        }

		for (auto [id, arb] : arbiters)
		{
            auto* dump_ptr = (collision_dump ? &(*collision_dump)["collisions"][ndx] : nullptr);
			arb.gather_and_solve_collisions(world, deltaTime, dump_ptr);
			ndx++;
		}

        // update attachments
        for (auto [id, col] : collidables) {
            auto& attach = world.at(col.get_attach_id());
            attach.teleport(col.getPrevPosition());
            attach.set_pos(col.getPosition());
            attach.set_vel(col.get_vel());
        }
	}

	for (auto [id, col] : collidables) {
		col.debug_draw();
	}
	collision_dump = nullptr;
	frame_count++;
};

void CollisionSystem::notify_created(World& world, ID<Collidable> id)
{
    arbiters[id] = CollidableArbiter{.collidable_id = id};
}

void CollisionSystem::notify_created(World& world, ID<ColliderRegion> id)
{
}

void CollisionSystem::notify_erased(World& world, ID<Collidable> id)
{
    arbiters.erase(id);
}

void CollisionSystem::notify_erased(World& world, ID<ColliderRegion> id)
{
    for (auto& [_, arb] : arbiters)
    {
        arb.erase_region(id);
    }
}

}
