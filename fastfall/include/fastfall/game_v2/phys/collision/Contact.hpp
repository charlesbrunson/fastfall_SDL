#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/game_v2/phys/collider_coretypes/ColliderSurface.hpp"
#include "fastfall/game_v2/tile/Tile.hpp"
#include "CollisionID.hpp"

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

struct Contact
{
	bool isResolvable() const noexcept;

	Vec2f getSurfaceVel() const;

	bool isTransposable() const noexcept;
	inline bool isTranposed() const { return is_transposed; };
	void transpose() noexcept;

	float separation = 0.f;

    ColliderSurface collider;

	Vec2f position;
	Vec2f ortho_n;
	Vec2f collider_n;

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

	bool isSlip = false;
    bool is_transposed = false;
    bool hasValley = false;
    bool hasContact = false;

    const SurfaceMaterial* material = nullptr;
    std::optional<CollisionID> id;

    secs touchDuration = 0.0;
    ContactType type = ContactType::NO_SOLUTION;
};

bool ContactCompare(const Contact* lhs, const Contact* rhs);

}
