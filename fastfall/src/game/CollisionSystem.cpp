#include "fastfall/game/CollisionSystem.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

#include <algorithm>
#include <execution>
#include <set>

#include "fastfall/game/phys/CollisionSolver.hpp"
#include "fastfall/game/WorldState.hpp"

#include "nlohmann/json.hpp"

namespace ff {

CollisionSystem::CollisionSystem(WorldState& st)
{
	st.push_listener(this);
}

void CollisionSystem::update(WorldState& st, secs deltaTime) {

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
		for (auto& collidable : st.list<Collidable>()) 
		{
			auto collidable_id = st.id(collidable);
			auto dump_ptr = (collision_dump ? &(*collision_dump)["collisions"][ndx] : nullptr);

			//collidable.update(deltaTime);
			gather_collisions(st, deltaTime, collidable_id, dump_ptr);
			solve_collisions(st, deltaTime, collidable_id, dump_ptr);
		}
	}

	for (auto& collidable : st.list<Collidable>()) {
		collidable.debug_draw();
	}
	collision_dump = nullptr;
	frame_count++;
};


void CollisionSystem::notify_component_created(WorldState& st, ID<Entity> entity, GenericID gid) {
	if (auto id = id_cast<Collidable>(gid))
	{
		collisions.insert(*id, {});
	}
}

void CollisionSystem::notify_component_destroyed(WorldState& st, ID<Entity> entity, GenericID gid) {
	if (auto id = id_cast<Collidable>(gid))
	{
		collisions.erase(*id);
	}
	else if (auto id = id_cast<ColliderRegion>(gid))
	{
		for (auto& collision : collisions)
		{
			std::erase_if(collision.second, [&id](const RegionArbiter& arb) {
					arb.getColliderID() == *id;
				});
		}
	}
}


inline void CollisionSystem::gather_collisions(
	WorldState& st,
	secs deltaTime,
	ID<Collidable> collidable_id,
	nlohmann::ordered_json* dump_ptr)
{
	auto& collidable = st.get(collidable_id);
	auto& regions = collisions.at(collidable_id);

	if (dump_ptr) {
		(*dump_ptr)["collidable"] = {
			{ "id",			collidable_id.raw()},
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

		update_region_arbiters(st, collidable_id, body_bound);

		// try to push out collidable bounds
		for (auto& regionArb : regions) {
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
		for (auto& rArb : regions) {
			auto collider_id = rArb.getColliderID();
			auto& collider = st.get(collider_id);

			(*dump_ptr)["broad_phase"]["colliders"][region_count] = {
				{ "id",				collider_id.raw()},
				{ "vel",			fmt::format("{}", collider.velocity)},
				{ "delta_pos",		fmt::format("{}", collider.getPosition() - collider.getPrevPosition()) },
				{ "arbiter_count",	rArb.getQuadArbiters().size()},
			};
			region_count++;
		}
	}
}


void CollisionSystem::solve_collisions(
	WorldState& st,
	secs deltaTime,
	ID<Collidable> collidable_id,
	nlohmann::ordered_json* dump_ptr)
{
	auto& collidable = st.get(collidable_id);
	auto& regions = collisions.at(collidable_id);

	CollisionSolver solver{ &collidable };

	for (auto& rArb : regions) {

		for (auto& qArb : rArb.getQuadArbiters())
		{
			auto& arb = qArb.second;
			auto& collider = st.get(rArb.getColliderID());

			// allow precontact callback to reject collision
			if (collider.on_precontact(arb.collider, *arb.getContactPtr(), arb.getTouchDuration()))
			{
				solver.pushContact(arb.getContactPtr());
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

			if (auto* arbiter = aContact.contact.arbiter)
			{
				pContact.touchDuration = arbiter->getTouchDuration();
				pContact.region = arbiter->region;


				if (const auto* collider = arbiter->collider)
				{
					pContact.quad_id = arbiter->collider->quad_id;
				}

				if (const auto* region = pContact.region)
				{
					pContact.collider_id = region->get_ID();
					region->on_postcontact(pContact);
				}
			}
			return pContact;
		});

	collidable.set_frame(std::move(c));
}


void CollisionSystem::update_region_arbiters(WorldState& st, ID<Collidable> collidable_id, Rectf bounds)
{

	auto& region_arbiters = collisions.at(collidable_id);
	for (auto& region : st.list<ColliderRegion>()) {

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

				RegionArbiter rarb(region.get(), &st.get(collidable_id));
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


Rectf CollisionSystem::push_bounds_for_contact(Rectf push_bound, const cardinal_array<float>&boundDist, const Contact * contact)
{
	if (!contact->hasContact || !direction::from_vector(contact->ortho_n).has_value())
		return push_bound;

	Cardinal dir = direction::from_vector(contact->ortho_n).value();
	Rectf push = push_bound;

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



}
