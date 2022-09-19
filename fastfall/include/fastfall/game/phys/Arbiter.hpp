#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/game/phys/collision/CollisionContinuous.hpp"
#include "fastfall/game/phys/collision/CollisionContext.hpp"

namespace ff {

class Arbiter {
private:
	CollisionContinuous collision;

	secs aliveTimer = 0.0; // time this arbiter has existed
	secs touchTimer = 0.0; // time of positive collision

	size_t recalcCounter = 0;

    void accumTime(secs deltaTime);

public:
	Arbiter(CollisionID t_id);

    void reset(CollisionContext ctx, secs deltaTime);
	void update(CollisionContext ctx, secs deltaTime);

	void setApplied();

	inline const CollisionContinuous* getCollision() const noexcept { return &collision; };

	inline ContinuousContact getContact() const noexcept { return collision.getContact(); };

	inline secs getAliveDuration() const noexcept { return aliveTimer; };
	inline secs getTouchDuration() const noexcept { return touchTimer; };
	inline size_t getRecalcCount() const noexcept { return recalcCounter; };

    CollisionID id;

	Rectf quad_bounds;

	bool stale = false;

};

}
