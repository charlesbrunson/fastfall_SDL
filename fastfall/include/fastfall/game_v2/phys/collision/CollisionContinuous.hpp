#pragma once

#include <memory>
#include <optional>

#include "fastfall/game_v2/phys/collision/CollisionDiscrete.hpp"

namespace ff {

class CollisionContinuous {
private:
	CollisionDiscrete currCollision;
	CollisionDiscrete prevCollision;

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
    CollisionContinuous(ID<Collidable> collidable, ID<ColliderRegion> colliderRegion, QuadID colliderQuad);

	void update(secs deltaTime);

	inline void updateContact() noexcept { currCollision.updateContact(); };

	inline Contact getContact() const noexcept { return contact; }

    ID<Collidable> collidable_id() const { return collidable; }
    ID<ColliderRegion> collider_id() const { return collider; }
    QuadID quad_id() const { return quad; }

	//inline bool tileValid() const noexcept { return cTile.hasAnySurface(); };
	inline void setAxisApplied(Vec2f ortho_normal) noexcept { currCollision.setAxisApplied(ortho_normal); }
};

}
