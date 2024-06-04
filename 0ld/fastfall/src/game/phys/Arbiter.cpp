#include "fastfall/game/phys/Arbiter.hpp"

#include "tracy/Tracy.hpp"

namespace ff {

Arbiter::Arbiter(CollisionContext ctx, CollisionID t_id)
    : collision(ctx, t_id)
    , id(t_id)
{
}

void Arbiter::setApplied() {
	collision.setAxisApplied(collision.getContact().ortho_n);
}

void Arbiter::update(CollisionContext ctx, secs deltaTime)
{
    ZoneScoped;
	if (deltaTime > 0.0) {
		recalcCounter = 0;
	}
	else {
		recalcCounter++;
	}

    collision.update(ctx, deltaTime);

    if (deltaTime > 0.0) {
        aliveTimer += deltaTime;
        if (!collision.getContact().hasContact) {
            touchTimer = 0.0;
        }
        else {
            touchTimer += deltaTime;
        }
    }
    collision.set_touch_duration(touchTimer);
}


}
