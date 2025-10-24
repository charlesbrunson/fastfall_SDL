#pragma once

#include <memory>
#include <optional>

#include "fastfall/game/phys/collision/CollisionDiscrete.hpp"
#include "fastfall/game/phys/collision/CollisionContext.hpp"

namespace ff {

class CollisionContinuous {
private:
	CollisionDiscrete currCollision;
	CollisionDiscrete prevCollision;

    ColliderQuad prev_quad;

	ContinuousContact contact;
	bool evaluated = false;
	int lastAxisCollided = -1;

	void evalContact(CollisionContext ctx, secs deltaTime);
	void slipUpdate(CollisionContext ctx);
	std::optional<ContinuousContact> getVerticalSlipContact(float leeway);

public:
    CollisionContinuous(CollisionContext ctx, CollisionID t_id);

	void update(CollisionContext ctx, secs deltaTime);

	void updateContact(const CollisionContext ctx) noexcept { currCollision.updateContact(ctx); };

    void set_touch_duration(const secs time) noexcept {
        contact.touchDuration = time;
    }

	ContinuousContact& getContact() noexcept { return contact; }

    CollisionID id;

	void setAxisApplied(const Vec2f ortho_normal) noexcept { currCollision.setAxisApplied(ortho_normal); }
};

}
