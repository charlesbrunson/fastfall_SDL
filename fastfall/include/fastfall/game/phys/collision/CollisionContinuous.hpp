#pragma once

#include <memory>

#include "CollisionDiscrete.hpp"

namespace ff {

class CollisionContinuous {
private:
	CollisionDiscrete currCollision;
	CollisionDiscrete prevCollision;

	int lastAxisCollided = -1;

	const Contact* copiedContact = nullptr;

	int evalContact(secs deltaTime);

	const ColliderQuad* cTile;
	const Collidable* cAble;
	const ColliderRegion* region;

	Contact contact;
	bool evaluated = false;

	Vec2f regionPosition;

	Vec2f velocity;

public:

	inline bool tileValid() const noexcept {
		return cTile && cTile->hasAnySurface();
	};


	CollisionContinuous(const Collidable* collidable, const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion);
	CollisionContinuous(CollisionContinuous&&) = default;
	CollisionContinuous(const CollisionContinuous&) = default;

	// advances the collision state to the next tick
	void update(secs deltaTime);

	inline void updateContact() noexcept { currCollision.updateContact(); };
	inline const Contact* getContactPtr() const noexcept { return &contact; };

	inline Contact getContact() const noexcept { return contact; }
	inline Contact getDiscreteContact() const noexcept { return currCollision.getContact(); }
	inline Contact getDiscretePreviousContact() const noexcept { return prevCollision.getContact(); }

	// attempts to "slip" onto another axis to avoid a harsher collision
	// example: landing on a ledge instead of hitting a wall
	Contact getSlipContact(Cardinal slipDir, float slipTolerance);

	int resolveType = -1;

	void setAxisApplied(Vec2f ortho_normal) noexcept {
		currCollision.setAxisApplied(ortho_normal);
	}

};

}