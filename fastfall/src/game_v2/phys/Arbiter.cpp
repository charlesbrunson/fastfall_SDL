#include "fastfall/game_v2/phys/Arbiter.hpp"

#include "fastfall/util/log.hpp"

#include <sstream>
#include <iomanip>

namespace ff {

Arbiter::Arbiter(CollisionID t_id)
    : collision(t_id)
    , id(t_id)
{
}

void Arbiter::setApplied() {
	collision.setAxisApplied(collision.getContact().ortho_n);
}

void Arbiter::reset(CollisionContext ctx, secs deltaTime) {
    recalcCounter = 0;
    collision.update(ctx, deltaTime);
    accumTime(deltaTime);
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
    accumTime(deltaTime);
}

void Arbiter::accumTime(secs deltaTime) {
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
