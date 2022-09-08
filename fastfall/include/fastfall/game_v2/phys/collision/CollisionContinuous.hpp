#pragma once

#include <memory>
#include <optional>

#include "fastfall/game_v2/phys/collision/CollisionDiscrete.hpp"

namespace ff {

class CollisionContinuous {
private:
	CollisionDiscrete currCollision;
	CollisionDiscrete prevCollision;

	ColliderQuad prevTile;
    ColliderQuad cTile;
    ID<Collidable> collidable;
    ID<ColliderRegion> region;

	Contact contact;
	bool evaluated = false;
	int lastAxisCollided = -1;
	Vec2f velocity;

	void evalContact(secs deltaTime);
	void slipUpdate();
	std::optional<Contact> getVerticalSlipContact(float leeway);

public:
	//CollisionContinuous(const Collidable* collidable, const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion, Arbiter* arbiter);
    CollisionContinuous(ID<Collidable> collidable, ColliderQuad collisionTile, ID<ColliderRegion> colliderRegion /*, Arbiter* arbiter*/);

	void update(secs deltaTime);

	inline void updateContact() noexcept { currCollision.updateContact(); };

	inline Contact getContact() const noexcept { return contact; }

	inline bool tileValid() const noexcept { return cTile.hasAnySurface(); };
	inline void setAxisApplied(Vec2f ortho_normal) noexcept { currCollision.setAxisApplied(ortho_normal); }
};

}
