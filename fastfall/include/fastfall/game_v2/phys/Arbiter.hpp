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
	Arbiter(ID<Collidable> collidable, ID<ColliderRegion> collider, QuadID quad);

	void update(secs deltaTime);

	void setApplied();

	inline const CollisionContinuous* getCollision() const noexcept { return &collision; };

	inline Contact getContact() const noexcept { return collision.getContact(); };

	inline secs getAliveDuration() const noexcept { return aliveTimer; };
	inline secs getTouchDuration() const noexcept { return touchTimer; };
	inline size_t getRecalcCount() const noexcept { return recalcCounter; };

    ID<Collidable> collidable_id;
    ID<ColliderRegion> collider_id;
    QuadID quad_id;

	Rectf quad_bounds;

	bool stale = false;

};

}
