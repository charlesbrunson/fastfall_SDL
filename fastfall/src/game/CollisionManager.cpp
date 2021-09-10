#include "fastfall/game/CollisionManager.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

#include <algorithm>
#include <execution>
#include <set>

#include "fastfall/game/phys/CollisionSolver.hpp"
#include "fastfall/game/Instance.hpp"

namespace ff {

CollisionManager::CollisionManager(unsigned instance) :
	instanceID(instance)
{

}

void CollisionManager::update(secs deltaTime) {
	if (deltaTime > 0.0) {
		broadPhase(deltaTime);

		for (auto& colData : collidables) {

			solve(colData);
		}
	}
};

// --------------------------------------------------------------

Collidable* CollisionManager::create_collidable() {
	auto it = collidables.emplace(Collidable{ Instance(InstanceID{instanceID})->getContext() });
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

	for (auto& colData : collidables) {
		Rectf body_rect(colData.collidable.getBox());

		// using double for this as float can lead to infinite loop due to floating point inaccuracy when pushing boundary
		Rectf body_bound(colData.collidable.getBoundingBox());
		Rectf push_bound(body_bound);

		std::vector<const Arbiter*> updatedBuffer;

		static auto is_updated = [](std::vector<const Arbiter*>& updated, const Arbiter* arbiter) {
			return std::find(updated.cbegin(), updated.cend(), arbiter) != updated.end();
		};

		do {
			body_bound = push_bound;
			std::array<float, 4u> boundDist = {
				body_rect.top - body_bound.top,												// NORTH
				(body_bound.left + body_bound.width) - (body_rect.left + body_rect.width),	// EAST
				(body_bound.top + body_bound.height) - (body_rect.top + body_rect.height),	// SOUTH
				body_rect.left - body_bound.left											// WEST
			};

			updateRegionArbiters(colData);

			// try to push out collidable bounds
			for (auto& regionArb : colData.regionArbiters) {
				for (auto& [quad, arbiter] : regionArb.getQuadArbiters()) {

					// if this arbiter hasn't pushed before, update it
					if (!is_updated(updatedBuffer, &arbiter)) {
						arbiter.update(deltaTime);
						updatedBuffer.push_back(&arbiter);
					}

					updatePushBound(push_bound, boundDist, arbiter.getContactPtr());

				}
			}

		} while (body_bound != push_bound);
	}
};

void CollisionManager::updateRegionArbiters(CollidableData& data) {

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
		if (region->getBoundingBox().intersects(data.collidable.getBoundingBox())) {

			if (!exists) {
				// just entered this region

				RegionArbiter rarb(region.get(), &data.collidable);
				arbiter = data.regionArbiters.emplace(arbiter, rarb);
			}
			arbiter->updateRegion(data.collidable.getBoundingBox());
		}
		else if (exists) {
			// just left this region
			data.regionArbiters.erase(arbiter);
		}
		
	}
}

void CollisionManager::updatePushBound(Rectf& push_bound, const std::array<float, 4u>& boundDist, const Contact* contact) {

	if (!contact->hasContact || !vecToCardinal(contact->ortho_normal).has_value())
		return;

	Cardinal dir = vecToCardinal(contact->ortho_normal).value();

	float diff = contact->separation - boundDist[dir];
	if (diff > 0.f) {
		push_bound = math::rect_extend(push_bound, dir, diff);
	}

	if (math::is_vertical(contact->ortho_normal) && contact->isTransposable()) {

		Vec2f alt_ortho_normal = Vec2f{ (contact->collider_normal.x < 0.f ? -1.f : 1.f), 0.f };
		auto alt_dir = vecToCardinal(alt_ortho_normal);

		if (alt_dir.has_value()) {
			//float alt_separation = abs((contact->collider_normal.y / contact->collider_normal.x) * contact->separation);

			float alt_separation = abs(contact->collider_normal.y * contact->separation * (1.f / contact->collider_normal.x));

			float alt_diff = alt_separation - boundDist[alt_dir.value()];

			if (alt_diff > 0.f) {
				push_bound = math::rect_extend(push_bound, alt_dir.value(), alt_diff);
			}
		}
	}
}

void CollisionManager::solve(CollidableData& collidableData) {

	CollisionSolver solver{ &collidableData.collidable };

	for (auto& rArb : collidableData.regionArbiters) {

		for (auto& qArb : rArb.getQuadArbiters()) {
			if (rArb.getRegion()->on_precontact(qArb.second.collider->getID(), *qArb.second.getContactPtr(), qArb.second.getTouchDuration())) {
				solver.pushArbiter(&qArb.second);
			}
		}
	}

	solver.solve();

	// push collision data to collidable
	std::vector<PersistantContact> c;
	for (auto& applied : solver.frame) {
		c.push_back(PersistantContact(applied.contact));
		auto& contact = c.back();

		contact.touchDuration = (applied.arbiter ? applied.arbiter->getTouchDuration() : 0.0);
		contact.type = applied.type;

		if (applied.region)
			contact.collider_id = applied.region->get_ID();

		if (applied.arbiter && applied.arbiter->collider)
			contact.quad_id = applied.arbiter->collider->getID();

		if (applied.region)
			applied.region->on_postcontact(contact.quad_id, contact);
		
	}
	collidableData.collidable.set_frame(std::move(c));
}

}
