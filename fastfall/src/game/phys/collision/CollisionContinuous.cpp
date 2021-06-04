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
		contact.hasContact = touchingAxis->is_intersecting();

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
			contact.hasContact = axis.is_intersecting();
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
		velocity = region->velocity;
	}

	if (contact.hasContact)
		contact.velocity = velocity;

	evaluated = true;

	if (deltaTime == 0.0) {
		slipUpdate();
	}

	return resolve;
}


void CollisionContinuous::slipUpdate() {

	// vertical slip
	if (cAble->getSlipV() != 0.f) {
		auto slip = getVerticalSlipContact(cAble->getSlipV());
		if (slip) {
			contact = slip.value();
		}
	}

	// horizontal slip
	// TODO
}

std::optional<Contact> CollisionContinuous::getVerticalSlipContact(float leeway) {
	// contact must be evaluated first
	if (!evaluated)
		throw "contact must be evaluated first";

	// leeway must be != 0.f
	if (leeway == 0.f)
		return std::nullopt;

	// leeway must be > 0
	if (leeway < 0.f)
		leeway *= -1.f;

	// slip only applicable on first contact
	if (!contact.hasContact  || !contact.hasImpactTime)
		return std::nullopt;

	// if we're trying to slip vertically then the 
	// contact must be horizontal
	if (contact.ortho_normal.y != 0.f)
		return std::nullopt;

	auto& cAxes = currCollision.getCollisionAxes();

	auto nAxis = std::find_if(cAxes.begin(), cAxes.end(), [](const auto& axis) {return axis.dir == Cardinal::NORTH; });
	auto sAxis = std::find_if(cAxes.begin(), cAxes.end(), [](const auto& axis) {return axis.dir == Cardinal::SOUTH; });

	bool canNorth = false;
	bool canSouth = false;

	if (nAxis->is_collider_valid() && nAxis != cAxes.end() && nAxis->contact.separation - leeway < 0.f) {
		canNorth = true;
	}
	if (sAxis->is_collider_valid() && sAxis != cAxes.end() && sAxis->contact.separation - leeway < 0.f) {
		canSouth = true;
	}

	else if (canNorth && canSouth) {
		LOG_INFO("SLIP BOTH");

		canNorth = sAxis->contact.separation > nAxis->contact.separation;
		canSouth = !canNorth;
	}

	if (canNorth && !canSouth) {
		LOG_INFO("SLIP NORTH");
		Contact c = nAxis->contact;
		c.isSlip = true;
		c.hasImpactTime = contact.hasImpactTime;
		c.impactTime = contact.impactTime;
		return c;
	}

	else if (!canNorth && canSouth) {
		LOG_INFO("SLIP SOUTH");
		Contact c = sAxis->contact;
		c.isSlip = true;
		c.hasImpactTime = contact.hasImpactTime;
		c.impactTime = contact.impactTime;
		return c;
	}

	return std::nullopt;
}

}