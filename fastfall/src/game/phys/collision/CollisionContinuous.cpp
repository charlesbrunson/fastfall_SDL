#include "fastfall/game/phys/collision/CollisionContinuous.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/math.hpp"

//#include <SFML/System.hpp>

namespace ff {

CollisionContinuous::CollisionContinuous(const Collidable* collidable, const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion) :
	cAble(collidable),
	prevCollision(collidable, collisionTile, colliderRegion, true),
	currCollision(collidable, collisionTile, colliderRegion, false),
	cTile(collisionTile),
	region(colliderRegion)
{
	evalContact(0.0);
}

void CollisionContinuous::update(secs deltaTime) {

	Vec2f prevPosition = region->getPrevPosition();

	if (deltaTime > 0.0) {
		if (prevPosition == regionPosition) {
			prevCollision = std::move(currCollision);
			prevCollision.setPrevious();
			prevCollision.updateContact();
		}
		else {
			prevCollision.reset(cTile, region, true);
		}

		currCollision.reset(cTile, region, false);
		resolveType = evalContact(deltaTime);
	}
	else {
		/*
		if (prevCollision.getCollisionAxes().size() !=
			currCollision.getCollisionAxes().size()) {

			prevCollision = CollisionDiscrete{ cAble, cTile, region, true };

		}
		*/

		currCollision.updateContact();
		evalContact(deltaTime);
	}
	regionPosition = region->getPosition();
}

int CollisionContinuous::evalContact(secs deltaTime) {
	auto getRoot = [](float y0, float y1) {
		return -1.f * (y0 / (y1 - y0));
	};

	contact = Contact{};
	copiedContact = nullptr;

	auto& pAxes = prevCollision.getCollisionAxes();
	auto& cAxes = currCollision.getCollisionAxes();

	assert(pAxes.size() == cAxes.size());

	float firstExit = 1.f;
	float lastEntry = 0.f;

	const CollisionAxis* touchingAxis = nullptr;

	int touchingAxisNdx = 0;

	bool alwaysColliding = true;
	bool noCollision = false;

	const CollisionAxis* pAxis;
	const CollisionAxis* cAxis;

	bool pIntersects;
	bool cIntersects;

	float root;
	int resolve = -1;

	assert(pAxes.size() == cAxes.size());

	for (size_t i = 0; i < cAxes.size(); i++) {
		pAxis = &pAxes.at(i);
		cAxis = &cAxes.at(i);

		pIntersects = pAxis->is_intersecting() && !pAxis->applied;
		cIntersects = cAxis->is_intersecting();

		assert(pAxis->dir == cAxis->dir);
		assert(pAxis->contact.ortho_normal == cAxis->contact.ortho_normal);

		if (!pIntersects && !cIntersects) {
			contact.hasContact = false;
			alwaysColliding = false;
			noCollision = true;
		}
		else if (pIntersects && !cIntersects) {
			alwaysColliding = false;

			assert(getRoot(pAxis->contact.separation, cAxis->contact.separation) >= 0.f);
			firstExit = std::min(firstExit, getRoot(pAxis->contact.separation, cAxis->contact.separation));

		}
		else if (!pIntersects && cIntersects) {
			root = getRoot(pAxis->contact.separation, cAxis->contact.separation);

			if (root >= 0.f && root < 1.f && root >= lastEntry) {
				alwaysColliding = false;

				if (touchingAxis && root == lastEntry) {
					if (touchingAxis->contact.separation > cAxis->contact.separation && cAxis->is_collider_valid()) {
						touchingAxis = cAxis;
						touchingAxisNdx = i;
					}
				}
				else {
					lastEntry = root;
					if (cAxis->is_collider_valid()) {
						touchingAxis = cAxis;
						touchingAxisNdx = i;
					}
				}
			}
		}
	}


	if (noCollision) {
		copiedContact = nullptr;
		contact = currCollision.getContact();

		resolve = -2;
	}
	else if (touchingAxis != nullptr && firstExit >= lastEntry) {
		copiedContact = &touchingAxis->contact;
		contact = touchingAxis->contact;
		contact.hasContact = touchingAxis->is_intersecting();// (touchingAxis->inclusive ? touchingAxis->contact.separation >= 0.f : touchingAxis->contact.separation > 0.f);

		if (lastEntry == 0.f && firstExit < 1.f) {
			contact.hasContact &= currCollision.getContact().hasContact;
		}

		contact.hasImpactTime = lastEntry > 0;
		contact.impactTime = lastEntry;
		lastAxisCollided = touchingAxisNdx;
		resolve = 1;
	}
	else if (firstExit < lastEntry) {
		copiedContact = nullptr;
		contact = currCollision.getContact();
		resolve = 2;
	}
	else {
		if (alwaysColliding && lastAxisCollided >= 0 && cAxes.at(lastAxisCollided).applied) {
			auto& axis = cAxes.at(lastAxisCollided);

			copiedContact = &axis.contact;
			contact = axis.contact;
			contact.hasContact = axis.is_intersecting(); //(axis.inclusive ? axis.contact.separation >= 0.f : axis.contact.separation > 0.f);
			resolve = 3;
		}
		else {
			copiedContact = nullptr;
			contact = currCollision.getContact();
			lastAxisCollided = -1;
			resolve = 4;
		}
	}

	if (deltaTime > 0.0) {
		//velocity = (region->getPosition() - region->getPrevPosition()) / deltaTime;
		velocity = region->velocity;
	}

	if (contact.hasContact)
		contact.velocity = velocity;

	evaluated = true;
	return resolve;
}

Contact CollisionContinuous::getSlipContact(Cardinal slipDir, float slipTolerance) {
	// TODO (?)
	return Contact{};
}

}