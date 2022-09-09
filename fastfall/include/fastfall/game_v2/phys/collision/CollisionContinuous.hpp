#pragma once

#include <memory>
#include <optional>

#include "fastfall/game_v2/phys/collision/CollisionDiscrete.hpp"
#include "fastfall/game_v2/phys/collision/CollisionContext.hpp"

namespace ff {

class CollisionContinuous {
private:
	CollisionDiscrete currCollision;
	CollisionDiscrete prevCollision;

    ColliderQuad prev_quad;

	Contact contact;
	bool evaluated = false;
	int lastAxisCollided = -1;
	Vec2f velocity;

	void evalContact(CollisionContext ctx, secs deltaTime);
	void slipUpdate(CollisionContext ctx);
	std::optional<Contact> getVerticalSlipContact(float leeway);

public:
    CollisionContinuous(CollisionID t_id);

	void update(CollisionContext ctx, secs deltaTime);

	inline void updateContact() noexcept { currCollision.updateContact(); };

	inline const Contact& getContact() const noexcept { return contact; }

    CollisionID id;

	//inline bool tileValid() const noexcept { return cTile.hasAnySurface(); };
	inline void setAxisApplied(Vec2f ortho_normal) noexcept { currCollision.setAxisApplied(ortho_normal); }
};

}
