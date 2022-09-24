#include "fastfall/game/phys/CollidableArbiter.hpp"

#include "fastfall/game/phys/CollisionSolver.hpp"
#include "fastfall/util/id_map.hpp"
#include "fastfall/game/World.hpp"

#include "nlohmann/json.hpp"

#include <algorithm>

namespace ff {
	void CollidableArbiter::erase_region(ID<ColliderRegion> id)
	{
        region_arbiters.erase(id);
	}

	void CollidableArbiter::gather_collisions(
            World& world,
			secs deltaTime,
			nlohmann::ordered_json* dump_ptr)
	{
        auto& collidable = world.at(collidable_id);

		if (dump_ptr) {
			(*dump_ptr)["collidable"] = {
				{ "id",			collidable_id.raw() },
				{ "pos",		fmt::format("{}", collidable.getPosition()) },
				{ "delta_pos",	fmt::format("{}", collidable.getPosition() - collidable.getPrevPosition()) },
				{ "vel",		fmt::format("{}", collidable.get_vel()) },
				{ "size",		fmt::format("{}", Vec2f{collidable.getBox().getSize()}) }
			};
		}

		std::set<const Arbiter*> updatedBuffer;

		Rectf body_rect(collidable.getBox());

		// using double for this as float can lead to infinite loop due to floating point inaccuracy when pushing boundary
		Rectf body_bound(collidable.getBoundingBox());
		Rectf push_bound(body_bound);

		do {
			body_bound = push_bound;
			cardinal_array<float> boundDist = {
				body_rect.top - body_bound.top,												// NORTH
				(body_bound.left + body_bound.width) - (body_rect.left + body_rect.width),	// EAST
				(body_bound.top + body_bound.height) - (body_rect.top + body_rect.height),	// SOUTH
				body_rect.left - body_bound.left											// WEST
			};

			update_region_arbiters(world, body_bound);

			// try to push out collidable bounds
			for (auto& [rid, rarb] : region_arbiters) {

                CollisionContext ctx{
                    .collider = world.get(rid),
                    .collidable = &collidable
                };

				for (auto& [quad, arbiter] : rarb.getQuadArbiters()) {

					// if this arbiter hasn't pushed before, update it
					if (!updatedBuffer.contains(&arbiter)) {
						arbiter.update(ctx, deltaTime);
						updatedBuffer.insert(&arbiter);
					}

					push_bound = push_bounds_for_contact(push_bound, boundDist, arbiter.getContact());

				}
			}

		} while (body_bound != push_bound);

		if (dump_ptr) 
		{
			(*dump_ptr)["broad_phase"]["init_bounds"] = {
				{"pos",  fmt::format("{}", Vec2f{collidable.getBoundingBox().getPosition()}) },
				{"size", fmt::format("{}", Vec2f{collidable.getBoundingBox().getSize()}) },
			};

			(*dump_ptr)["broad_phase"]["final_bounds"] = {
				{"pos",  fmt::format("{}", Vec2f{body_bound.getPosition()}) },
				{"size", fmt::format("{}", Vec2f{body_bound.getSize()}) },
			};

			size_t region_count = 0;
			for (auto& [rid, rarb] : region_arbiters) {
                ColliderRegion* region = world.get(rid);
				(*dump_ptr)["broad_phase"]["colliders"][region_count] = {
					{ "id",				rid.raw()},
					{ "vel",			fmt::format("{}", region->velocity)},
					{ "delta_pos",		fmt::format("{}", region->getPosition() - region->getPrevPosition()) },
					{ "arbiter_count",	rarb.getQuadArbiters().size()},
				};
				region_count++;
			}
		}
	}

	void CollidableArbiter::update_region_arbiters(World& world, Rectf bounds)
	{
        auto& collidable = world.at(collidable_id);
		for (auto& region : world.all<ColliderRegion>()) {

            auto collider_id = world.all<ColliderRegion>().id_of(region);
            auto rarb_iter = region_arbiters.find(collider_id);
            bool exists = rarb_iter != region_arbiters.end();

			// check if collidable is in this region
			if (region->getSweptBoundingBox().intersects(bounds)) {
				if (!exists) {
					// just entered this region
					rarb_iter = region_arbiters.emplace(collider_id, RegionArbiter{ collider_id, collidable_id }).first;
				}
				rarb_iter->second.updateRegion({ region.get(), &collidable }, bounds);
			}
			else if (exists) {
				// just left this region
				region_arbiters.erase(collider_id);
			}
			
		}
	}

	Rectf CollidableArbiter::push_bounds_for_contact(Rectf init_push_bound, const cardinal_array<float>& boundDist, const ContinuousContact& contact)
	{
		if (!contact.hasContact || !direction::from_vector(contact.ortho_n).has_value())
			return init_push_bound;

		Cardinal dir = direction::from_vector(contact.ortho_n).value();
		Rectf push = init_push_bound;

		float diff = contact.separation - boundDist[dir];
		if (diff > 0.f) {
			push = math::rect_extend(push, dir, diff);
		}

		if (math::is_vertical(contact.ortho_n) && contact.transposable())
		{
			Vec2f alt_ortho_normal = Vec2f{ (contact.collider_n.x < 0.f ? -1.f : 1.f), 0.f };
			auto alt_dir = direction::from_vector(alt_ortho_normal);

			if (alt_dir.has_value()) 
			{
				float alt_separation = abs(contact.collider_n.y * contact.separation * (1.f / contact.collider_n.x));
				float alt_diff = alt_separation - boundDist[alt_dir.value()];

				if (alt_diff > 0.f) 
				{
					push = math::rect_extend(push, alt_dir.value(), alt_diff);
				}
			}
		}
		return push;
	}

	void CollidableArbiter::solve_collisions(
            World& world,
            nlohmann::ordered_json* dump_ptr)
	{
        auto& colliders = world.all<ColliderRegion>();
        auto& collidable = world.at(collidable_id);
		CollisionSolver solver{ &colliders, &collidable };

		for (auto& [rid, rarb] : region_arbiters) {

            auto& collider = colliders.at(rid);
			for (auto& [qid, qarb] : rarb.getQuadArbiters())
			{
                // allow precontact callback to reject collision
				if (collider.on_precontact(world, qarb.getContact(), qarb.getTouchDuration()))
				{
					solver.pushContact(&qarb);
				}
			}
		}

		nlohmann::ordered_json* json_dump = nullptr;
		if (dump_ptr) {
			json_dump = &(*dump_ptr)["solver"];
		}

        collidable.set_frame(&colliders, solver.solve(json_dump));

        if (collidable.callbacks.onPostCollision)
            collidable.callbacks.onPostCollision(world);
	}

}
