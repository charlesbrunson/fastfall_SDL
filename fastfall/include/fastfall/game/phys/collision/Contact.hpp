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

class ColliderRegion;
class Arbiter;

struct Contact 
{
	bool isResolvable() const noexcept;

	Vec2f getSurfaceVel() const;

	bool isTransposable() const noexcept;
	inline bool isTranposed() const { return is_transposed; };
	void transpose() noexcept;

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


	const SurfaceMaterial* material = nullptr;
	Arbiter* arbiter = nullptr;


	bool is_transposed = false;

};

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

bool ContactCompare(const Contact* lhs, const Contact* rhs);

}
