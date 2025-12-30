#include "fastfall/game/systems/CollisionSystem.hpp"

#include "fastfall/game/phys/CollisionSolver.hpp"
#include "fastfall/game/World.hpp"

#include <algorithm>

#include "nlohmann/json.hpp"
#include "tracy/Tracy.hpp"

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
        {
            ZoneScopedN("Update Colliders");
            for (auto [id, collider]: colliders) {
                collider->update(deltaTime);
            }
        }

        size_t ndx = 0;
        {
            ZoneScopedN("Update Collidables");
            for (auto [id, col]: collidables) {
                col.update(&colliders, deltaTime);
            }
        }

        {
            ZoneScopedN("Solve Collisions");
            for (auto [id, arb]: arbiters) {
                auto *dump_ptr = (collision_dump ? &(*collision_dump)["collisions"][ndx] : nullptr);
                arb.gather_and_solve_collisions(world, deltaTime, dump_ptr);
                ndx++;
            }
        }

        {
            ZoneScopedN("Update Collidable Attachpoints");
            // update attachments
            for (auto [id, col]: collidables) {
                auto attach_id = col.get_attach_id();
                auto &attach = world.at(attach_id);
                attach.set_pos(col.getPosition() + col.get_attach_origin());
                attach.set_parent_vel(col.get_global_vel());
                world.system<AttachSystem>().update_attachments(world, attach_id, deltaTime);
            }
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
