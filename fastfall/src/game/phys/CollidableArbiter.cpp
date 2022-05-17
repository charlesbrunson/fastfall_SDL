#include "fastfall/game/phys/CollidableArbiter.hpp"
#include "fastfall/game/phys/CollisionSolver.hpp"

#include "nlohmann/json.hpp"

namespace ff {
	void CollidableArbiter::erase_region(const ColliderRegion* region)
	{
		region_arbiters.erase(
			std::remove_if(region_arbiters.begin(), region_arbiters.end(),
				[region](const RegionArbiter& arb) -> bool {
					return arb.getRegion() == region;
				}
			),
			region_arbiters.end()
		);
	}

	void CollidableArbiter::gather_collisions(
			secs deltaTime, 
			const std::vector<std::unique_ptr<ColliderRegion>>& regions,
			nlohmann::ordered_json* dump_ptr)
	{
		if (dump_ptr) {
			(*dump_ptr)["collidable"] = {
				{ "id",			collidable.get_ID().value },
				{ "pos",		fmt::format("{}", collidable.getPosition()) },
				{ "delta_pos",	fmt::format("{}", collidable.getPosition() - collidable.getPrevPosition()) },
				{ "vel",		fmt::format("{}", collidable.get_vel()) },
				{ "size",		fmt::format("{}", Vec2f{collidable.getBox().getSize()}) }
			};
		}

		std::set<const Arbiter*> updatedBuffer;

		auto is_updated = [&](const Arbiter& arbiter) {
			return std::find(updatedBuffer.cbegin(), updatedBuffer.cend(), &arbiter) != updatedBuffer.end();
		};

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

			update_region_arbiters(body_bound, regions);

			// try to push out collidable bounds
			for (auto& regionArb : region_arbiters) {
				for (auto& [quad, arbiter] : regionArb.getQuadArbiters()) {

					// if this arbiter hasn't pushed before, update it
					if (!updatedBuffer.contains(&arbiter)) {
						arbiter.update(deltaTime);
						updatedBuffer.insert(&arbiter);
					}

					push_bound = push_bounds_for_contact(push_bound, boundDist, arbiter.getContactPtr());

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
			for (auto& rArb : region_arbiters) {
				(*dump_ptr)["broad_phase"]["colliders"][region_count] = {
					{ "id",				rArb.getRegion()->get_ID().value},
					{ "vel",			fmt::format("{}", rArb.getRegion()->velocity)},
					{ "delta_pos",		fmt::format("{}", rArb.getRegion()->getPosition() - rArb.getRegion()->getPrevPosition()) },
					{ "arbiter_count",	rArb.getQuadArbiters().size()},
				};
				region_count++;
			}
		}
	}

	void CollidableArbiter::update_region_arbiters(Rectf bounds, const std::vector<std::unique_ptr<ColliderRegion>>& regions)
	{
		for (const auto& region : regions) {

			auto arbiter = std::lower_bound(
				region_arbiters.begin(),
				region_arbiters.end(),
				region,
				[](const RegionArbiter& arb, const std::unique_ptr<ColliderRegion>& region) {
					return arb.getRegion() < region.get();
				}
			);

			bool exists = arbiter != region_arbiters.end() && arbiter->getRegion() == region.get();

			// check if collidable is in this region
			if (region->getSweptBoundingBox().intersects(bounds)) {

				if (!exists) {
					// just entered this region

					RegionArbiter rarb(region.get(), &collidable);
					arbiter = region_arbiters.emplace(arbiter, rarb);
				}
				arbiter->updateRegion(bounds);
			}
			else if (exists) {
				// just left this region
				region_arbiters.erase(arbiter);
			}
			
		}
	}

	Rectf CollidableArbiter::push_bounds_for_contact(Rectf init_push_bound, const cardinal_array<float>& boundDist, const Contact* contact) 
	{
		if (!contact->hasContact || !direction::from_vector(contact->ortho_n).has_value())
			return init_push_bound;

		Cardinal dir = direction::from_vector(contact->ortho_n).value();
		Rectf push = init_push_bound;

		float diff = contact->separation - boundDist[dir];
		if (diff > 0.f) {
			push = math::rect_extend(push, dir, diff);
		}

		if (math::is_vertical(contact->ortho_n) && contact->isTransposable()) 
		{
			Vec2f alt_ortho_normal = Vec2f{ (contact->collider_n.x < 0.f ? -1.f : 1.f), 0.f };
			auto alt_dir = direction::from_vector(alt_ortho_normal);

			if (alt_dir.has_value()) 
			{
				float alt_separation = abs(contact->collider_n.y * contact->separation * (1.f / contact->collider_n.x));
				float alt_diff = alt_separation - boundDist[alt_dir.value()];

				if (alt_diff > 0.f) 
				{
					push = math::rect_extend(push, alt_dir.value(), alt_diff);
				}
			}
		}
		return push;
	}

	void CollidableArbiter::solve_collisions(nlohmann::ordered_json* dump_ptr) 
	{
		CollisionSolver solver{ &collidable };
		//solver.frame_count = 0;

		for (auto& rArb : region_arbiters) {

			for (auto& qArb : rArb.getQuadArbiters()) {
				// allow precontact callback to reject collision
				if (rArb.getRegion()->on_precontact(qArb.second.collider->getID(), *qArb.second.getContactPtr(), qArb.second.getTouchDuration())) 
				{
					solver.pushContact(qArb.second.getContactPtr());
				}
			}
		}

		nlohmann::ordered_json* json_dump = nullptr;
		if (dump_ptr) {

			json_dump = &(*dump_ptr)["solver"];
		}
		solver.solve(json_dump);

		// push collision data to collidable
		std::vector<PersistantContact> c;
		for (auto& applied : solver.frame) {
			c.push_back(PersistantContact(applied.contact));

			auto& contact = c.back();
			auto* arbiter = applied.contact.arbiter;

			contact.type = applied.type;
			contact.touchDuration = (arbiter ? arbiter->getTouchDuration() : 0.0);

			if (arbiter)
			{
				auto* region = arbiter->region;

				contact.collider_id = region->get_ID();
				contact.region = region;

				if (region)
				{
					region->on_postcontact(contact.quad_id, contact);
				}
			}

			if (arbiter && arbiter->collider)
				contact.quad_id = arbiter->collider->getID();
					
		}
		collidable.set_frame(std::move(c));
		//frame_collision_count++;
	}

}
