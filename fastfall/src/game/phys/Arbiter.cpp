#include "fastfall/game/phys/Arbiter.hpp"

//#include "collision/CollisionQuad.hpp"

#include "fastfall/util/log.hpp"

#include <sstream>
#include <iomanip>

namespace ff {

Arbiter::Arbiter(Collidable* col_dable, const ColliderQuad* col_der, const ColliderRegion* col_region) :
	collision(col_dable, col_der, col_region)
{
	collidable = col_dable;
	collider = col_der;
	region = col_region;
}


void Arbiter::setApplied() {
	collision.setAxisApplied(collision.getContact().ortho_normal);
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

}