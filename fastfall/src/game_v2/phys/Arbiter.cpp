#include "fastfall/game_v2/phys/Arbiter.hpp"

#include "fastfall/util/log.hpp"

#include <sstream>
#include <iomanip>

namespace ff {

Arbiter::Arbiter(ID<Collidable> collidable, ID<ColliderRegion> collider, QuadID quad)
    : collision(collidable, collider, quad)
{
    collidable_id = collidable;
    collider_id = collider;
    quad_id = quad;
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

}
