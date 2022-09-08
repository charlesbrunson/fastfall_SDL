#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/game_v2/phys/collider_coretypes/ColliderSurface.hpp"

#include "Contact.hpp"

namespace ff {

struct AxisPreStep {
	Cardinal dir = Cardinal::N;
	ColliderSurface surface;
	bool is_real = false;
	bool is_valid = false;
	uint8_t quadNdx = 255u;
};

class CollisionAxis {
public:
	CollisionAxis(const AxisPreStep& initData) 
		: dir(initData.dir)
		, quadIndex(initData.quadNdx)
		, collider_real(initData.is_real)
		, collider_valid(initData.is_valid)
	{
		contact.collider = initData.surface;
	}

	CollisionAxis()
	{
	}

	// the contact on this axis
	Contact contact;

	// the index of this surface in the original quad
	uint8_t quadIndex = 255u;

	// direction of the surface
	Cardinal dir = Cardinal::N;

	// flag to invalidate entire collision
	bool axisValid = true;

	// resolution was applied on this axis
	bool applied = false;

	// cached calcuations for contact updating
	float separationOffset = 0.f;


	// this axis can be used for resolving a collision
	inline bool is_collider_valid() const noexcept {
		return collider_valid;
	}

	// the collider from this axis exists, and wasn't generated (ie. corners are generated)
	inline bool is_collider_real() const noexcept {
		return collider_real;
	}

	inline bool is_intersecting() const noexcept {
		return (collider_valid ? contact.separation > 0.f : contact.separation >= 0.f);
	}

private:
	// this surface is exterior
	bool collider_valid = false;

	// surface was not generated (i.e. not a corner)
	bool collider_real = false;
};

}
