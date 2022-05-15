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


bool ContactCompare(const Contact* lhs, const Contact* rhs)
{
	//const Contact& aC = a->getContact();
	//const Contact& bC = b->getContact();

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

	// favor oldest contact
	/*
	if (lhs->arbiter && rhs->arbiter
		&& lhs->arbiter->getAliveDuration() != rhs->arbiter->getAliveDuration()) 
	{
		return lhs->arbiter->getAliveDuration() > rhs->arbiter->getAliveDuration();
	}
	*/

	return false;
};

}