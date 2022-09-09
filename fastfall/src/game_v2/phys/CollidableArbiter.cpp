#include "fastfall/game_v2/phys/CollidableArbiter.hpp"
#include "fastfall/game_v2/phys/CollisionSolver.hpp"
#include "fastfall/util/id_map.hpp"
#include "fastfall/game_v2/World.hpp"

#include "nlohmann/json.hpp"

#include <algorithm>

namespace ff {
	void CollidableArbiter::erase_region(ID<ColliderRegion> id)
	{
		region_arbiters.erase(
			std::remove_if(region_arbiters.begin(), region_arbiters.end(),
				[id](const RegionArbiter& arb) -> bool {
					return arb.get_collider_id() == id;
				}
			),
			region_arbiters.end()
		);
	}

	void CollidableArbiter::gather_collisions(
            Collidable& collidable,
            poly_id_map<ColliderRegion>& colliders,
			secs deltaTime,
			nlohmann::ordered_json* dump_ptr)
	{

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

			update_region_arbiters(collidable, colliders, body_bound);

			// try to push out collidable bounds
			for (auto& regionArb : region_arbiters) {

                CollisionContext ctx{
                    .collider = colliders.get(regionArb.get_collider_id()),
                    .collidable = &collidable
                };

				for (auto& [quad, arbiter] : regionArb.getQuadArbiters()) {

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
			for (auto& rArb : region_arbiters) {
                ColliderRegion* region = colliders.get(rArb.get_collider_id());
				(*dump_ptr)["broad_phase"]["colliders"][region_count] = {
					{ "id",				rArb.get_collider_id().raw()},
					{ "vel",			fmt::format("{}", region->velocity)},
					{ "delta_pos",		fmt::format("{}", region->getPosition() - region->getPrevPosition()) },
					{ "arbiter_count",	rArb.getQuadArbiters().size()},
				};
				region_count++;
			}
		}
	}

	void CollidableArbiter::update_region_arbiters(Collidable& collidable, poly_id_map<ColliderRegion>& colliders, Rectf bounds)
	{
		for (auto& region : colliders) {

            auto collider_id = colliders.id_of(region);

			auto arbiter = std::lower_bound(
				region_arbiters.begin(),
				region_arbiters.end(),
				collider_id,
				[](const RegionArbiter& arb, ID<ColliderRegion> region_id) {
					return arb.get_collider_id() < region_id;
				}
			);

			bool exists = arbiter != region_arbiters.end() && arbiter->get_collider_id() == collider_id;

			// check if collidable is in this region
			if (region->getSweptBoundingBox().intersects(bounds)) {
				if (!exists) {
					// just entered this region
					arbiter = region_arbiters.emplace(arbiter, RegionArbiter{ collider_id, collidable_id });
				}
				arbiter->updateRegion({ region.get(), &collidable }, bounds);
			}
			else if (exists) {
				// just left this region
				region_arbiters.erase(arbiter);
			}
			
		}
	}

	Rectf CollidableArbiter::push_bounds_for_contact(Rectf init_push_bound, const cardinal_array<float>& boundDist, Contact contact)
	{
		if (!contact.hasContact || !direction::from_vector(contact.ortho_n).has_value())
			return init_push_bound;

		Cardinal dir = direction::from_vector(contact.ortho_n).value();
		Rectf push = init_push_bound;

		float diff = contact.separation - boundDist[dir];
		if (diff > 0.f) {
			push = math::rect_extend(push, dir, diff);
		}

		if (math::is_vertical(contact.ortho_n) && contact.isTransposable())
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
            Collidable& collidable,
            poly_id_map<ColliderRegion>& colliders,
            nlohmann::ordered_json* dump_ptr)
	{
		CollisionSolver solver{ &collidable };

		for (auto& rArb : region_arbiters) {

            auto& collider = colliders.at(rArb.get_collider_id());
			for (auto& qArb : rArb.getQuadArbiters())
			{
                // allow precontact callback to reject collision
				auto& arb = qArb.second;
				if (collider.on_precontact(arb.id.quad, arb.getContact(), arb.getTouchDuration()))
				{
					solver.pushContact(arb.getContact());
				}
			}
		}

		nlohmann::ordered_json* json_dump = nullptr;
		if (dump_ptr) {
			json_dump = &(*dump_ptr)["solver"];
		}
		auto frame = solver.solve(json_dump);

		// push collision data to collidable
		std::vector<PersistantContact> c;

		std::transform(frame.cbegin(), frame.cend(), std::back_inserter(c), 
			[](const AppliedContact& aContact) -> PersistantContact {
				PersistantContact pContact{ aContact.contact };
				pContact.type = aContact.type;


                // TODO get arbiter
                Arbiter* arb = world->collision().get_arbiter(collidable_id);
				if (arb)
				{
					pContact.touchDuration = arbiter->getTouchDuration();
					pContact.region = arbiter->region;
                    pContact.quad_id = arbiter->quad;

					if (const auto* region = world->get(pContact.region))
					{
						pContact.region = region->id();
						region->on_postcontact(pContact);
					}
				}
				return pContact;
			});

		collidable.set_frame(std::move(c));
	}

}
