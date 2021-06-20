#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderSurface.hpp"

#include "fastfall/game/phys/Identity.hpp"

#include <string>

namespace ff {

enum class ContactType : unsigned char {
	NO_SOLUTION,

	// typical case
	// from a single arbiter
	SINGLE,

	// wedge solution
	// derived from two arbiters
	WEDGE_SAME,
	WEDGE_OPPOSITE,
	WEDGE_WALL,

	// crush solution
	// derived from two arbiters
	CRUSH_HORIZONTAL,
	CRUSH_VERTICAL
};
const char* contactTypeToString(ContactType t);

struct Contact {

	inline bool isResolvable() const noexcept {
		return ortho_normal != Vec2f();
	}

	inline bool isTransposable() const noexcept {
		return abs(collider_normal.x) > abs(collider_normal.y) &&
			hasImpactTime && !hasValley;
	}

	float separation = 0.f;
	bool hasContact = false;

	Vec2f position;
	Vec2f ortho_normal;
	Vec2f collider_normal;
	ColliderSurface collider;

	bool hasValley = false;

	// the velocity of the surface in contact
	// relative to worldspace
	Vec2f velocity;

	// the velocity of the surface
	float surfaceVel = 0.f;
	Vec2f getSurfaceVel() const { return collider_normal.righthand() * surfaceVel; }

	// moment that the object started intersecting the collider,
	// represented as a fraction of the tick deltatime [0, 1.0]
	// used by continuous collision
	float impactTime = -1.0;
	bool  hasImpactTime = false;
	bool  isSlip = false;

};


struct PersistantContact : public Contact {
	PersistantContact(Contact c) :
		Contact{ c }
	{

	}

	secs touchDuration = 0.0;
	ContactType type = ContactType::NO_SOLUTION;

	ColliderID collider_id = ColliderID{ ColliderID::NO_ID };
	int quad_id = 0u;
};

}