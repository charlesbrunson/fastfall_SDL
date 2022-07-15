#pragma once

#include "Collidable.hpp"
#include "collision/Contact.hpp"
#include "fastfall/game/phys/collision/CollisionContinuous.hpp"

namespace ff {

class Arbiter {
private:
	CollisionContinuous collision;

	secs aliveTimer = 0.0; // time this arbiter has existed
	secs touchTimer = 0.0; // time of positive collision

	size_t recalcCounter = 0;

public:
	Arbiter(Collidable* col_dable, const ColliderQuad* col_der, const ColliderRegion* col_region);
	
	Arbiter(const Arbiter&);
	Arbiter(Arbiter&&) noexcept;

	Arbiter& operator=(const Arbiter&);
	Arbiter& operator=(Arbiter&&) noexcept;

	void update(secs deltaTime);

	void setApplied();

	inline const CollisionContinuous& getCollision() const noexcept { return collision; };

	inline Contact getContact() const noexcept { return collision.getContact(); };
	inline Contact* getContactPtr() noexcept { return collision.getContactPtr(); };

	inline secs getAliveDuration() const noexcept { return aliveTimer; };
	inline secs getTouchDuration() const noexcept { return touchTimer; };
	inline size_t getRecalcCount() const noexcept { return recalcCounter; };

	//Collidable* collidable;
	//const ColliderQuad* collider;
	//const ColliderRegion* region;

	ID<Collidable> collidable;
	ID<ColliderRegion> collider;
	QuadID quad;

	Rectf quad_bounds;

	bool stale = false;

};

}
