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

bool compare_contact(const ContinuousContact& lhs, const ContinuousContact& rhs) {

    // favor valid contact
    if (lhs.hasContact != rhs.hasContact) {
        return lhs.hasContact;
    }

    // favor contact with impact time
    if (lhs.hasImpactTime != rhs.hasImpactTime) {
        return lhs.hasImpactTime;
    }

    // favor earliest impact time
    if (lhs.hasImpactTime && rhs.hasImpactTime && lhs.impactTime != rhs.impactTime) {
        return lhs.impactTime < rhs.impactTime;
    }

    // favor least separation
    if (lhs.separation != rhs.separation) {
        return lhs.separation < rhs.separation;
    }

    // favor unmoving contact
    float aVelMag = lhs.velocity.magnitudeSquared();
    float bVelMag = rhs.velocity.magnitudeSquared();
    if (aVelMag != bVelMag) {
        return aVelMag < bVelMag;
    }

    return false;
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