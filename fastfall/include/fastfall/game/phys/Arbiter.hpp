#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/game/phys/collision/CollisionContinuous.hpp"
#include "fastfall/game/phys/collision/CollisionContext.hpp"

namespace ff {

class Arbiter {
	CollisionContinuous collision;

	secs aliveTimer = 0.0; // time this arbiter has existed
	secs touchTimer = 0.0; // time of positive collision
	size_t recalcCounter = 0;

public:
	Arbiter(CollisionContext ctx, CollisionID t_id);

	void update(CollisionContext ctx, secs deltaTime);

	void setApplied();

	[[nodiscard]] const CollisionContinuous* getCollision() const noexcept { return &collision; };

	ContinuousContact& getContact() noexcept { return collision.getContact(); };

	[[nodiscard]] secs getAliveDuration() const noexcept { return aliveTimer; };
	[[nodiscard]] secs getTouchDuration() const noexcept { return touchTimer; };
	[[nodiscard]] size_t getRecalcCount() const noexcept { return recalcCounter; };

    CollisionID id;

	bool stale = false;

};

}
