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
		broadPhase(deltaTime);

		for (auto& colData : collidables) {

			solve(colData);
		}
	}
	for (auto& colData : collidables) {
		colData.collidable.debug_draw();
	}
	collision_dump = nullptr;
	frame_count++;
	frame_collision_count = 0u;
};

// --------------------------------------------------------------

Collidable* CollisionManager::create_collidable() {
	auto it = collidables.emplace(Collidable{});
	return &it->collidable;
}
Collidable* CollisionManager::create_collidable(Vec2f init_pos, Vec2f init_size, Vec2f init_grav) {
	Collidable* col = create_collidable();
	col->init(init_pos, init_size, init_grav);
	return col;
}

bool CollisionManager::erase_collidable(Collidable* collidable) {

	auto it = std::find_if(collidables.begin(), collidables.end(), 
		[&collidable](const CollidableData& data) {
			return collidable == &data.collidable;
		});

	if (it != collidables.end()) {
		collidables.erase(it);
		collidable = nullptr;
		return true;
	}
	else {
		return false;
	}
	return false;
}

bool CollisionManager::erase_collider(ColliderRegion* region) {

	for (auto& collidableData : collidables) {
		collidableData.regionArbiters.erase(
			std::remove_if(collidableData.regionArbiters.begin(), collidableData.regionArbiters.end(),
				[region](const RegionArbiter& arb) -> bool {
					return arb.getRegion() == region;
				}
			),
			collidableData.regionArbiters.end()
		);
	}

	auto it = std::find_if(regions.begin(), regions.end(),
		[&region](const std::unique_ptr<ColliderRegion>& rptr) {
			return rptr.get() == region;
		});

	if (it != regions.end()) {
		regions.erase(it);
		region = nullptr;
		return true;
	}
	else {
		return false;
	}
}

// --------------------------------------------------------------

void CollisionManager::broadPhase(secs deltaTime) {

	std::vector<const Arbiter*> updatedBuffer;

	auto is_updated = [&](const Arbiter& arbiter) {
		return std::find(updatedBuffer.cbegin(), updatedBuffer.cend(), &arbiter) != updatedBuffer.end();
	};

	for (auto& colData : collidables) 
	{
		Rectf body_rect(colData.collidable.getBox());

		// using double for this as float can lead to infinite loop due to floating point inaccuracy when pushing boundary
		Rectf body_bound(colData.collidable.getBoundingBox());
		Rectf push_bound(body_bound);

		do {
			body_bound = push_bound;
			cardinal_array<float> boundDist = {
				body_rect.top - body_bound.top,												// NORTH
				(body_bound.left + body_bound.width) - (body_rect.left + body_rect.width),	// EAST
				(body_bound.top + body_bound.height) - (body_rect.top + body_rect.height),	// SOUTH
				body_rect.left - body_bound.left											// WEST
			};

			updateRegionArbiters(colData, body_bound);

			// try to push out collidable bounds
			for (auto& regionArb : colData.regionArbiters) {
				for (auto& [quad, arbiter] : regionArb.getQuadArbiters()) {

					// if this arbiter hasn't pushed before, update it
					if (!is_updated(arbiter)) {
						arbiter.update(deltaTime);
						updatedBuffer.push_back(&arbiter);
					}

					push_bound = updatePushBound(push_bound, boundDist, arbiter.getContactPtr());

				}
			}

		} while (body_bound != push_bound);

		updatedBuffer.clear();
	}
};

void CollisionManager::updateRegionArbiters(CollidableData& data, Rectf bounds) {

	for (const auto& region : regions) {

		auto arbiter = std::lower_bound(
			data.regionArbiters.begin(),
			data.regionArbiters.end(),
			region,
			[](const RegionArbiter& arb, const std::unique_ptr<ColliderRegion>& region) {
				return arb.getRegion() < region.get();
			}
		);

		bool exists = arbiter != data.regionArbiters.end() && arbiter->getRegion() == region.get();

		// check if collidable is in this region
		if (region->getSweptBoundingBox().intersects(bounds)) {

			if (!exists) {
				// just entered this region

				RegionArbiter rarb(region.get(), &data.collidable);
				arbiter = data.regionArbiters.emplace(arbiter, rarb);
			}
			arbiter->updateRegion(bounds);
		}
		else if (exists) {
			// just left this region
			data.regionArbiters.erase(arbiter);
		}
		
	}
}

Rectf CollisionManager::updatePushBound(Rectf init_push_bound, const cardinal_array<float>& boundDist, const Contact* contact) {

	if (!contact->hasContact || !direction::from_vector(contact->ortho_n).has_value())
		return init_push_bound;

	Cardinal dir = direction::from_vector(contact->ortho_n).value();
	Rectf push = init_push_bound;

	float diff = contact->separation - boundDist[dir];
	if (diff > 0.f) {
		push = math::rect_extend(push, dir, diff);
	}

	if (math::is_vertical(contact->ortho_n) && contact->isTransposable()) {

		Vec2f alt_ortho_normal = Vec2f{ (contact->collider_n.x < 0.f ? -1.f : 1.f), 0.f };
		auto alt_dir = direction::from_vector(alt_ortho_normal);

		if (alt_dir.has_value()) {
			//float alt_separation = abs((contact->collider_n.y / contact->collider_n.x) * contact->separation);

			float alt_separation = abs(contact->collider_n.y * contact->separation * (1.f / contact->collider_n.x));

			float alt_diff = alt_separation - boundDist[alt_dir.value()];

			if (alt_diff > 0.f) {
				push = math::rect_extend(push, alt_dir.value(), alt_diff);
			}
		}
	}
	return push;
}

void CollisionManager::solve(CollidableData& collidableData) {

	CollisionSolver solver{ &collidableData.collidable };
	solver.frame_count = frame_count;

	for (auto& rArb : collidableData.regionArbiters) {

		for (auto& qArb : rArb.getQuadArbiters()) {
			// allow precontact callback to reject collision
			if (rArb.getRegion()->on_precontact(qArb.second.collider->getID(), *qArb.second.getContactPtr(), qArb.second.getTouchDuration())) 
			{
				solver.pushContact(qArb.second.getContactPtr());
			}
		}
	}

	nlohmann::ordered_json* json_dump = nullptr;
	if (collision_dump) {

		auto* entry = &(*collision_dump)["collisions"][frame_collision_count];

		const auto& col = collidableData.collidable;

		(*entry)["collidable"] = {
			{ "id",		col.get_ID().value },
			{ "pos",		fmt::format("{}", col.getPosition()) },
			{ "delta_pos",	fmt::format("{}", col.getPosition() - col.getPrevPosition()) },
			{ "vel",		fmt::format("{}", col.get_vel()) },
			{ "size",		fmt::format("{}", Vec2f{col.getBox().getSize()}) }
		};

		size_t region_count = 0;
		for (auto& rArb : collidableData.regionArbiters) {
			(*entry)["colliders"][region_count] = {
				{ "id",				rArb.getRegion()->get_ID().value},
				{ "vel",			fmt::format("{}", rArb.getRegion()->velocity)},
				{ "delta_pos",		fmt::format("{}", rArb.getRegion()->getPosition() - rArb.getRegion()->getPrevPosition()) },
				{ "arbiter_count",	rArb.getQuadArbiters().size()},
			};
			region_count++;
		}

		json_dump = &(*entry)["solver"];
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
	collidableData.collidable.set_frame(std::move(c));
	frame_collision_count++;
}

}
