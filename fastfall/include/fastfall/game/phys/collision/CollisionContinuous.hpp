#pragma once

#include <memory>
#include <optional>

#include "fastfall/game/phys/collision/CollisionDiscrete.hpp"

namespace ff {

class CollisionContinuous {
private:
	CollisionDiscrete currCollision;
	CollisionDiscrete prevCollision;

	ColliderQuad prevTile;

	ID<Collidable> collidable;
	ID<ColliderRegion> collider;
	QuadID quad;

	Contact contact;
	bool evaluated = false;
	int lastAxisCollided = -1;
	Vec2f velocity;

	void evalContact(secs deltaTime);
	void slipUpdate();
	std::optional<Contact> getVerticalSlipContact(float leeway);


public:
	CollisionContinuous(const Collidable* collidable, const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion, Arbiter* arbiter);

	void update(secs deltaTime);

	inline void updateContact() noexcept { currCollision.updateContact(); };

	inline Contact* getContactPtr() noexcept { return &contact; };
	inline Contact getContact() const noexcept { return contact; }

	inline bool tileValid() const noexcept { 
		return quad.valid() && cTile->hasAnySurface();
	};
	inline void setAxisApplied(Vec2f ortho_normal) noexcept { currCollision.setAxisApplied(ortho_normal); }

};

}
