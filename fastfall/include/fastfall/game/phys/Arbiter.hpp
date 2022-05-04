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
	//Arbiter(Arbiter&& arb) noexcept;
	//Arbiter& operator= (Arbiter&& arb) noexcept;

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

	Rectf quad_bounds;

	bool stale = false;

};

bool ArbiterCompare(const Arbiter* a, const Arbiter* b);

}
