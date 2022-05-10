#pragma once

#include "fastfall/game/level/Tile.hpp"
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
	WEDGE,

	// crush solution
	// derived from two arbiters
	CRUSH_HORIZONTAL,
	CRUSH_VERTICAL
};
std::string_view contactTypeToString(ContactType t);

struct Contact {

	inline bool isResolvable() const noexcept {
		return ortho_n != Vec2f();
	}

	inline bool isTransposable() const noexcept {
		return std::abs(collider_n.x) > std::abs(collider_n.y) 
			&& hasImpactTime 
			&& !hasValley;
	}

	inline Vec2f getSurfaceVel() const {
		return (material ? collider_n.righthand() * material->velocity : Vec2f{});
	}

	float separation = 0.f;
	bool hasContact = false;

	Vec2f position;
	Vec2f ortho_n;
	Vec2f collider_n;
	ColliderSurface collider;

	bool hasValley = false;

	// the velocity of the surface in contact
	// relative to worldspace
	Vec2f velocity;
		
	const SurfaceMaterial* material = nullptr;
	
	// offset to stick to the surface after the contact
	// multiply with ortho_normal
	float stickOffset = 0.f;
	Linef stickLine;

	// moment that the object started intersecting the collider,
	// represented as a fraction of the tick deltatime [0, 1.0]
	// used by continuous collision
	float impactTime = -1.0;
	bool  hasImpactTime = false;


	bool  isSlip = false;
};

class ColliderRegion;

struct PersistantContact : public Contact {
	PersistantContact(Contact c) :
		Contact{ c }
	{

	}

	secs touchDuration = 0.0;
	ContactType type = ContactType::NO_SOLUTION;

	ColliderID collider_id = ColliderID{ ColliderID::NO_ID };
	const ColliderRegion* region = nullptr;
	int quad_id = 0u;
};

}
