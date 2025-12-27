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

std::weak_ordering compare_contact(const ContinuousContact& lhs, const ContinuousContact& rhs) {

    // favor valid contact

    /*
    if (lhs.hasContact != rhs.hasContact) {
        return lhs.hasContact ? std::weak_ordering::less : std::weak_ordering::greater;
    }
    */

    // favor centered contact if floor/ceiling
    if (math::is_vertical(lhs.ortho_n) && math::is_vertical(rhs.ortho_n)
        && lhs.on_center != rhs.on_center)
    {
        return lhs.on_center ? std::weak_ordering::less : std::weak_ordering::greater;
    }

    // favor contact with impact time
    if (lhs.ortho_n != rhs.ortho_n && lhs.hasImpactTime != rhs.hasImpactTime) {
        return lhs.hasImpactTime ? std::weak_ordering::less : std::weak_ordering::greater;
    }

    // favor the earliest impact time
    if (lhs.hasImpactTime && rhs.hasImpactTime && lhs.impactTime != rhs.impactTime) {
        return lhs.impactTime < rhs.impactTime ? std::weak_ordering::less : std::weak_ordering::greater;
    }

    // favor the least separation
    if (lhs.separation != rhs.separation) {
        return lhs.separation < rhs.separation ? std::weak_ordering::less : std::weak_ordering::greater;
    }

    // favor unmoving contact
    float aVelMag = math::magnitude2(lhs.velocity);
    float bVelMag = math::magnitude2(rhs.velocity);
    if (aVelMag != bVelMag) {
        return aVelMag < bVelMag ? std::weak_ordering::less : std::weak_ordering::greater;
    }

    return std::weak_ordering::equivalent;
}

/*
bool ContinuousContact::operator< (const ContinuousContact& lhs, const ContinuousContact& rhs) const
{
	// favor valid contact
	if (hasContact != other.hasContact) {
		return hasContact;
	}

	// favor contact with impact time
	if (hasImpactTime != other.hasImpactTime) {
		return hasImpactTime;
	}

	// favor earliest impact time
	if (hasImpactTime && other.hasImpactTime && impactTime != other.impactTime) {
		return impactTime < other.impactTime;
	}

	// favor least separation
	if (separation != other.separation) {
		return separation < other.separation;
	}

	// favor unmoving contact
	float aVelMag = velocity.magnitudeSquared();
	float bVelMag = other.velocity.magnitudeSquared();
	if (aVelMag != bVelMag) {
		return aVelMag < bVelMag;
	}

	return false;
};
*/

}