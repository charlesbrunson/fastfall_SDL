#include "fastfall/game/CollisionManager.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

#include <algorithm>
#include <set>

//#include "Engine.hpp"

//#include "phys/collision/Collision.hpp"
//#include "phys/collision/CollisionQuad.hpp"

#include "fastfall/game/phys/CollisionSolver.hpp"

namespace ff {

CollisionManager::CollisionManager(unsigned instance) :
	instanceID(instance)
{

}


void CollisionManager::addColliderRegion(std::shared_ptr<ColliderRegion> col) {
	if (col->attached != nullptr) {
		colliders.colliders_attached.push_back(col);

		for (auto collidable = collidables.collidables_free.begin(); collidable != collidables.collidables_free.end(); collidable++) {
			if (&*collidable == col->attached) {
				collidables.collidables_attached.push_back(*collidable);
				collidables.collidables_free.erase(collidable);
				break;
			}
		}
	}
	else {
		colliders.colliders_free.push_back(col);
	}
};

Collidable* CollisionManager::createCollidable(Collidable&& col) {

	Collidable* ptr = nullptr;

	for (auto& collider : colliders.colliders_attached) {
		if (auto c_lock = collider.lock()) {
			if (c_lock->attached == &col) {
				collidables.collidables_attached.push_back(col);
				ptr = &collidables.collidables_attached.back();
			}
		}
	}
	if (!ptr) {
		collidables.collidables_free.push_back(col);
		ptr = &collidables.collidables_free.back();
	}
	assert(ptr);
	arbiters.insert(std::make_pair(ptr, ArbiterData{ ptr }));

	return ptr;

}

void CollisionManager::removeCollidable(Collidable* colptr) {
	assert(colptr != nullptr);

	arbiters.erase(colptr);

	collidables.collidables_free.remove_if([&](const Collidable& _Other) { return &_Other == colptr; });
	collidables.collidables_attached.remove_if([&](const Collidable& _Other) { return &_Other == colptr; });
}


void CollisionManager::update(secs deltaTime) {
	if (deltaTime > 0.0) {
		broadPhase(colliders.colliders_free, collidables.collidables_free, deltaTime);

		for (auto& [cptr, arbData] : arbiters) {

			solve(arbData);
		}
	}

	cleanInvalidCollisionObjects();
};


void CollisionManager::cleanInvalidCollisionObjects() {

	const static auto cleanseCollider = [this](std::vector<ColliderRegion_Wptr>& vec) {
		for (auto it = vec.begin(); it != vec.end(); ) {
			if (!it->lock()) {
				for (auto& arb : arbiters) {
					arb.second.regions.erase(*it);
				}
				it = vec.erase(it);
			}
			else {
				it++;
			}
		}
	};
	/*
	const static auto cleanseCollable = [this](std::list<Collidable>& vec) {
		for (auto it = vec.begin(); it != vec.end(); ) {
			if (!it->lock()) {
				arbiters.erase(*it);
				it = vec.erase(it);
			}
			else {
				it++;
			}
		}
	};
	*/

	cleanseCollider(colliders.colliders_free);
	cleanseCollider(colliders.colliders_attached);
	//cleanseCollable(collidables.collidables_free);
	//cleanseCollable(collidables.collidables_attached);
}


void CollisionManager::broadPhase(std::vector<ColliderRegion_Wptr>& p_colliders, std::list<Collidable>& p_collidables, secs deltaTime) {

	std::vector<std::pair<Rectf, const ColliderQuad*>> buffer;
	buffer.reserve(32);

	for (auto& body : p_collidables) {

		Rectf body_rect( body.getBox() );

		// using double for this as float can lead to infinite loop due to floating point inaccuracy when pushing boundary
		Rect<double> body_bound(body.getBoundingBox());
		Rect<double> push_bound(body_bound);

		ArbiterMap::iterator body_it{ arbiters.find(&body) };

		// this should've been created when the collidable was added
		if (body_it == arbiters.end()) {
			LOG_WARN("Could not find arbiter for collidable");
			continue;
		}

		RegionArbiterMap* collMap = &body_it->second.regions;

		std::vector<const Arbiter*> updatedBuffer;

		static auto is_updated = [](std::vector<const Arbiter*>& updated, const Arbiter* arbiter) {
			return std::find(updated.cbegin(), updated.cend(), arbiter) != updated.end();
		};

		do {
			body_bound = push_bound;
			std::array<double, 4u> boundDist = {
				body_rect.top - body_bound.top, // NORTH
				(body_bound.left + body_bound.width) - (body_rect.left + body_rect.width), // EAST
				(body_bound.top + body_bound.height) - (body_rect.top + body_rect.height), // SOUTH
				body_rect.left - body_bound.left // WEST
			};

			updateRegionArbiters(p_colliders, collMap, body);

			// try to push out collidable bounds
			for (auto& [region_wptr, regionArb] : *collMap) {
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

	contacts.clear();

};

void CollisionManager::updateRegionArbiters(std::vector<ColliderRegion_Wptr>& p_colliders, RegionArbiterMap* collMap, Collidable& collidable) {
	for (auto& coll_wptr : p_colliders) {
		if (auto coll = coll_wptr.lock()) {

			RegionArbiterMap::iterator coll_it = collMap->find(coll);

			// check if collidable is in this region
			if (coll->getBoundingBox().intersects(collidable.getBoundingBox())) {

				if (coll_it == collMap->end()) {
					// just entered this region

					RegionArbiter rarb(coll.get(), &collidable);
					coll_it = collMap->insert(std::make_pair(coll_wptr, std::move(rarb))).first;
				}

				coll_it->second.updateRegion(collidable.getBoundingBox());

			}
			else if (coll_it != collMap->end()) {
				// just left this region
				collMap->erase(coll_it);
			}
		}
	}
}

void CollisionManager::updatePushBound(Rect<double>& push_bound, const std::array<double, 4u>& boundDist, const Contact* contact) {

	if (!contact->hasContact || !vecToCardinal(contact->ortho_normal).has_value())
		return;

	Cardinal dir = vecToCardinal(contact->ortho_normal).value();

	double diff = (double)contact->separation - boundDist[dir];
	if (diff > 0.0) {
		push_bound = math::rect_extend(push_bound, dir, diff);
	}

	if (math::is_vertical(contact->ortho_normal) && contact->isTransposable()) {

		Vec2f alt_ortho_normal = Vec2f{ (contact->collider_normal.x < 0.f ? -1.f : 1.f), 0.f };
		auto alt_dir = vecToCardinal(alt_ortho_normal);

		if (alt_dir.has_value()) {
			double alt_separation = abs(((double)contact->collider_normal.y / (double)contact->collider_normal.x) * contact->separation);
			double alt_diff = alt_separation - boundDist[alt_dir.value()];

			if (alt_diff > 0.0) {
				push_bound = math::rect_extend(push_bound, alt_dir.value(), alt_diff);
			}
		}
	}
}

void CollisionManager::solve(ArbiterData& arbData) {

	CollisionSolver solver{ arbData.collidable };

	for (auto& rArb : arbData.regions) {

		if (auto ptr = rArb.first.lock()) {
			for (auto& qArb : rArb.second.getQuadArbiters()) {
				if (ptr->on_precontact(*qArb.second.getContactPtr(), qArb.second.getTouchDuration())) {
					solver.pushArbiter(&qArb.second);
				}
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
			applied.region->on_postcontact(contact);
		
	}
	arbData.collidable->set_frame(std::move(c));
}

}