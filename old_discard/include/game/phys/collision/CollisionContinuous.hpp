#pragma once

#include <memory>
#include <optional>

#include "CollisionDiscrete.hpp"

namespace ff {

class CollisionContinuous {
private:
	CollisionDiscrete currCollision;
	CollisionDiscrete prevCollision;

	ColliderQuad prevTile;
	const ColliderQuad* cTile;
	const Collidable* cAble;
	const ColliderRegion* region;

	Contact contact;
	bool evaluated = false;
	int lastAxisCollided = -1;
	Vec2f velocity;

	void evalContact(secs deltaTime);
	void slipUpdate();
	std::optional<Contact> getVerticalSlipContact(float leeway);

	Arbiter* arbiter = nullptr;

public:
	CollisionContinuous(const Collidable* collidable, const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion, Arbiter* arbiter);

	void update(secs deltaTime);

	inline void updateContact() noexcept { currCollision.updateContact(); };
	inline Contact* getContactPtr() noexcept { return &contact; };

	inline Contact getContact() const noexcept { return contact; }
	//inline Contact getDiscreteContact() const noexcept { return currCollision.getContact(); }
	//inline Contact getDiscretePreviousContact() const noexcept { return prevCollision.getContact(); }

	inline bool tileValid() const noexcept { return cTile && cTile->hasAnySurface(); };
	inline void setAxisApplied(Vec2f ortho_normal) noexcept { currCollision.setAxisApplied(ortho_normal); }

	void setArbiter(Arbiter* arb) {
		arbiter = arb;
		contact.arbiter = arb;
		prevCollision.setArbiter(arb);
		currCollision.setArbiter(arb);
	}
	inline Arbiter* getArbiter() { return arbiter; }
};

}