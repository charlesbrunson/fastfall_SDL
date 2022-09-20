#include "fastfall/game/phys/Arbiter.hpp"

#include "fastfall/util/log.hpp"

#include <sstream>
#include <iomanip>

namespace ff {

Arbiter::Arbiter(CollisionContext ctx, ColliderQuad quad, CollisionID t_id)
    : collision(ctx, t_id)
    , id(t_id)
{
}

void Arbiter::setApplied() {
	collision.setAxisApplied(collision.getContact().ortho_n);
}

void Arbiter::update(CollisionContext ctx, secs deltaTime)
{
	if (deltaTime > 0.0) {
		recalcCounter = 0;
	}
	else {
		recalcCounter++;
	}

    collision.update(ctx, deltaTime);

    if (deltaTime > 0.0) {
        aliveTimer += deltaTime;
        if (collision.getContact().hasContact) {
            touchTimer += deltaTime;
        }
        else {
            touchTimer = 0.0;
        }
    }

    collision.set_touch_duration(touchTimer);
}


}
