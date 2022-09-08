#pragma once

#include "fastfall/game_v2/phys/Collidable.hpp"
#include "fastfall/game_v2/phys/collision/Contact.hpp"
#include "fastfall/game_v2/phys/collision/CollisionContinuous.hpp"

namespace ff {

class Arbiter {
private:
	CollisionContinuous collision;

	secs aliveTimer = 0.0; // time this arbiter has existed
	secs touchTimer = 0.0; // time of positive collision

	size_t recalcCounter = 0;

public:
	Arbiter(ID<Collidable> col_dable, ColliderQuad col_der, ID<ColliderRegion> col_region);
	
	Arbiter(const Arbiter&);
	Arbiter(Arbiter&&) noexcept;

	Arbiter& operator=(const Arbiter&);
	Arbiter& operator=(Arbiter&&) noexcept;

	void update(secs deltaTime);

	void setApplied();

	inline const CollisionContinuous* getCollision() const noexcept { return &collision; };

	inline Contact getContact() const noexcept { return collision.getContact(); };

	inline secs getAliveDuration() const noexcept { return aliveTimer; };
	inline secs getTouchDuration() const noexcept { return touchTimer; };
	inline size_t getRecalcCount() const noexcept { return recalcCounter; };

    ID<Collidable> collidable;
    ID<ColliderRegion> region;
    ColliderQuad collider;

	Rectf quad_bounds;

	bool stale = false;

};

}
