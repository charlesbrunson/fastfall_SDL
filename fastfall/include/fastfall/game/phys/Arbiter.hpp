#pragma once

#include "Collidable.hpp"

#include "collision/Contact.hpp"

#include "collision/CollisionContinuous.hpp"

namespace ff {

class Arbiter {
private:
	CollisionContinuous collision;

	secs aliveTimer = 0.0; // time this arbiter has existed
	secs touchTimer = 0.0; // time of positive collision

	size_t recalcCounter = 0;

public:
	Arbiter(Collidable* col_dable, const ColliderQuad* col_der, const ColliderRegion* col_region);
	Arbiter(Arbiter&& arb) noexcept;

	//void update(Contact* newContact);
	void update(secs deltaTime);

	//void preStep(float inv_dt);
	void setApplied();

	inline const CollisionContinuous* getCollision() const noexcept { return &collision; };

	inline Contact getContact() const noexcept { return collision.getContact(); };
	inline const Contact* getContactPtr() const noexcept { return collision.getContactPtr(); };

	inline secs getAliveDuration() const noexcept { return aliveTimer; };
	inline secs getTouchDuration() const noexcept { return touchTimer; };
	inline size_t getRecalcCount() const noexcept { return recalcCounter; };

	Collidable* collidable;
	const ColliderQuad* collider;
	const ColliderRegion* region;

	bool stale = false;

};

inline bool ArbiterCompare(const Arbiter* a, const Arbiter* b)
{
	const Contact& aC = a->getContact();
	const Contact& bC = b->getContact();

	// favor valid contact
	if (aC.hasContact != bC.hasContact) {
		return aC.hasContact && !bC.hasContact;
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

}