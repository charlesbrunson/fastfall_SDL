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

	//Vec2f prevPosition = region->getPrevPosition();
	//regionPosition = region->getPosition();

	if (deltaTime > 0.0) {

		// check if conditions are similar enough we can reuse the
		// collision data from last frame
		if (region->getPrevPosition() == region->getPosition()
			&& prevTile == *cTile)
		{
			prevCollision = std::move(currCollision);
			prevCollision.setPrevious();
			prevCollision.updateContact();
		}
		// else redo the collision from last frame
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
	prevTile = *cTile;
}

int CollisionContinuous::evalContact(secs deltaTime) {
	auto getRoot = [](float y0, float y1) {
		return -1.f * (y0 / (y1 - y0));
	};

	contact = Contact{};
	copiedContact = nullptr;

	unsigned pCount = prevCollision.getAxisCount();
	unsigned cCount = currCollision.getAxisCount();

	assert(pCount == cCount);

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

	for (unsigned i = 0; i < cCount; i++) {
		pAxis = &prevCollision.getCollisionAxis(i);
		cAxis = &currCollision.getCollisionAxis(i);

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

	bool hasTouchAxis = touchingAxis != nullptr;
	bool intersectionOccurs = firstExit >= lastEntry;

	if (noCollision) {
		// no collision at all
		copiedContact = nullptr;
		contact = currCollision.getContact();

		resolve = -2;
	}
	else if (hasTouchAxis && intersectionOccurs) {

		// collision

		copiedContact = &touchingAxis->contact;
		contact = touchingAxis->contact;
		contact.hasContact = touchingAxis->is_intersecting();

		
		auto opposite = direction::opposite(touchingAxis->dir);
		bool opposite_intersecting = false;
		for (int ndx = 0; ndx < currCollision.getAxisCount(); ndx++)
		{
			if (ndx == touchingAxisNdx)
				continue;

			if (currCollision.getCollisionAxis(ndx).dir == opposite
				&& currCollision.getCollisionAxis(ndx).contact.separation > 0)
			{
				opposite_intersecting = true;
				break;
			}
		}

		// if the collidable is no longer intersecting the collider,
		// yet hasn't completely passed through this collider on this axis
		// require current collision has contact
		if (lastEntry == 0.f && firstExit < 1.f && opposite_intersecting)
		{
			contact.hasContact &= currCollision.getContact().hasContact;
		}
		

		contact.hasImpactTime = lastEntry > 0;
		contact.impactTime = lastEntry;
		lastAxisCollided = touchingAxisNdx;
		resolve = 1;
	}
	else if (!intersectionOccurs) {
		copiedContact = nullptr;
		contact = currCollision.getContact();
		resolve = 2;
	}
	else {
		if (alwaysColliding && lastAxisCollided >= 0 && currCollision.getCollisionAxis(lastAxisCollided).applied) {
			auto& axis = currCollision.getCollisionAxis(lastAxisCollided);

			copiedContact = &axis.contact;
			contact = axis.contact;
			contact.hasContact = axis.is_intersecting();
			resolve = 3;
		}
		else {
			copiedContact = nullptr;
			contact = currCollision.getContact();
			contact.hasContact &= 
				(math::is_horizontal(contact.ortho_normal) && abs(contact.separation) < currCollision.cAble->getBox().width)
				|| (abs(contact.separation) < currCollision.cAble->getBox().height);
			lastAxisCollided = -1;
			resolve = 4;
		}
	}

	if (resolve >= 0) {
		//fmt::print(stderr, "resolve={}\n", resolve);
		//LOG_INFO("resolve={}", resolve);
	}

	if (deltaTime > 0.0) {
		velocity = region->velocity;
	}

	//if (contact.hasContact)
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

	const CollisionAxis* nAxis = nullptr;
	const CollisionAxis* sAxis = nullptr;
	for (unsigned ndx = 0u; ndx < currCollision.getAxisCount(); ndx++) {
		auto& axis = currCollision.getCollisionAxis(ndx);
		if (!nAxis && axis.dir == Cardinal::N) {
			nAxis = &axis;
		}
		else if (!sAxis && axis.dir == Cardinal::S) {
			sAxis = &axis;
		}
	}

	bool canNorth = false;
	bool canSouth = false;

	if (nAxis && nAxis->is_collider_valid() && nAxis->contact.separation - leeway < 0.f) {
		canNorth = true;
	}
	if (sAxis && sAxis->is_collider_valid() && sAxis->contact.separation - leeway < 0.f) {
		canSouth = true;
	}

	if (canNorth && canSouth) {
		// select based on separation
		canNorth = sAxis->contact.separation > nAxis->contact.separation;
		canSouth = !canNorth;
	}

	if (canNorth && !canSouth) {
		Contact c = nAxis->contact;
		c.isSlip = true;
		c.hasImpactTime = contact.hasImpactTime;
		c.impactTime = contact.impactTime;
		return c;
	}
	else if (!canNorth && canSouth) {
		Contact c = sAxis->contact;
		c.isSlip = true;
		c.hasImpactTime = contact.hasImpactTime;
		c.impactTime = contact.impactTime;
		return c;
	}

	return std::nullopt;
}

}
