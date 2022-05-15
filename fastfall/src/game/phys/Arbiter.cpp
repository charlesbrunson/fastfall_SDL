#include "fastfall/game/phys/Arbiter.hpp"

//#include "collision/CollisionQuad.hpp"

#include "fastfall/util/log.hpp"

#include <sstream>
#include <iomanip>

namespace ff {

Arbiter::Arbiter(Collidable* col_dable, const ColliderQuad* col_der, const ColliderRegion* col_region) :
	collision(col_dable, col_der, col_region, this)
{
	collidable = col_dable;
	collider = col_der;
	region = col_region;
}


Arbiter::Arbiter(const Arbiter& rhs)
	: collision(rhs.collision)
	, collidable(rhs.collidable)
	, collider(rhs.collider)
	, region(rhs.region)
{
	aliveTimer = rhs.aliveTimer;
	touchTimer = rhs.touchTimer;
	recalcCounter = rhs.recalcCounter;

	quad_bounds = rhs.quad_bounds;
	stale = rhs.stale;

	collision.setArbiter(this);
}


Arbiter::Arbiter(Arbiter&& rhs) noexcept
	: collision(std::move(rhs.collision))
	, collidable(rhs.collidable)
	, collider(rhs.collider)
	, region(rhs.region)
{
	aliveTimer = rhs.aliveTimer;
	touchTimer = rhs.touchTimer;
	recalcCounter = rhs.recalcCounter;

	quad_bounds = rhs.quad_bounds;
	stale = rhs.stale;

	collision.setArbiter(this);
}

Arbiter& Arbiter::operator=(const Arbiter& rhs)
{
	collision = rhs.collision;

	aliveTimer = rhs.aliveTimer;
	touchTimer = rhs.touchTimer;
	recalcCounter = rhs.recalcCounter;

	collidable = rhs.collidable;
	collider = rhs.collider;
	region = rhs.region;

	quad_bounds = rhs.quad_bounds;
	stale = rhs.stale;

	collision.setArbiter(this);
	return *this;
}

Arbiter& Arbiter::operator=(Arbiter&& rhs) noexcept
{
	collision = std::move(rhs.collision);

	aliveTimer = rhs.aliveTimer;
	touchTimer = rhs.touchTimer;
	recalcCounter = rhs.recalcCounter;

	collidable = rhs.collidable;
	collider = rhs.collider;
	region = rhs.region;

	quad_bounds = rhs.quad_bounds;
	stale = rhs.stale;

	collision.setArbiter(this);
	return *this;
}


void Arbiter::setApplied() {
	collision.setAxisApplied(collision.getContact().ortho_n);
}

void Arbiter::update(secs deltaTime) {

	if (deltaTime > 0.0) {
		recalcCounter = 0;
	}
	else {
		recalcCounter++;
	}

	collision.update(deltaTime);
	//collision = CollisionDiscrete{ collidable, collider, region };

	if (deltaTime > 0.0) {
		aliveTimer += deltaTime;
		if (collision.getContact().hasContact) {
			touchTimer += deltaTime;
		}
		else {
			touchTimer = 0.0;
		}
	}
}

/*
bool ArbiterCompare(const Arbiter* a, const Arbiter* b)
{
	const Contact& aC = a->getContact();
	const Contact& bC = b->getContact();

	// favor valid contact
	if (aC.hasContact != bC.hasContact) {
		return aC.hasContact;
	}

	// favor contact with impact time
	if (aC.hasImpactTime != bC.hasImpactTime) {
		return aC.hasImpactTime;
	}

	// favor earliest impact time
	if (aC.hasImpactTime && bC.hasImpactTime && aC.impactTime != bC.impactTime) {
		return aC.impactTime < bC.impactTime;
	}

	// favor least separation
	if (aC.separation != bC.separation) {
		return aC.separation < bC.separation;
	}

	// favor unmoving contact
	float aVelMag = aC.velocity.magnitudeSquared();
	float bVelMag = bC.velocity.magnitudeSquared();
	if (aVelMag != bVelMag) {
		return aVelMag < bVelMag;
	}

	// favor oldest contact
	if (a->getAliveDuration() != b->getAliveDuration()) {
		return a->getAliveDuration() > b->getAliveDuration();
	}

	return false;
};
*/

}
