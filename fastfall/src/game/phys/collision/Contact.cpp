#include "fastfall/game/phys/collision/Contact.hpp"

#include "fastfall/game/phys/Arbiter.hpp"

namespace ff {

std::string_view contactTypeToString(ContactType t) {
	static const std::string_view types[] = {
		"No solution",
		"Single",
		"Wedge",
		"Crush horizontal",
		"Crush vertical"
	};
	return types[static_cast<unsigned char>(t)];
};


bool Contact::isResolvable() const noexcept {
	return ortho_n != Vec2f();
}

bool Contact::isTransposable() const noexcept {
	return !is_transposed
		&& math::is_vertical(ortho_n)
		&& std::abs(collider_n.x) > std::abs(collider_n.y)
		&& hasImpactTime
		&& !hasValley;
}

Vec2f Contact::getSurfaceVel() const {
	return (material ? collider_n.righthand() * material->velocity : Vec2f{});
}


void Contact::transpose() noexcept 
{
	if (!is_transposed) 
	{
		Vec2f alt_ortho_normal = (collider_n.x < 0.f ? Vec2f(-1.f, 0.f) : Vec2f(1.f, 0.f));
		float alt_separation = abs((collider_n.y * separation) / collider_n.x);

		ortho_n = alt_ortho_normal;
		separation = alt_separation;
		is_transposed = true;
	}
}



bool ContactCompare(const Contact* lhs, const Contact* rhs)
{
	// favor valid contact
	if (lhs->hasContact != rhs->hasContact) {
		return lhs->hasContact;
	}

	// favor contact with impact time
	if (lhs->hasImpactTime != rhs->hasImpactTime) {
		return lhs->hasImpactTime;
	}

	// favor earliest impact time
	if (lhs->hasImpactTime && rhs->hasImpactTime && lhs->impactTime != rhs->impactTime) {
		return lhs->impactTime < rhs->impactTime;
	}

	// favor least separation
	if (lhs->separation != rhs->separation) {
		return lhs->separation < rhs->separation;
	}

	// favor unmoving contact
	float aVelMag = lhs->velocity.magnitudeSquared();
	float bVelMag = rhs->velocity.magnitudeSquared();
	if (aVelMag != bVelMag) {
		return aVelMag < bVelMag;
	}

	return false;
};

}