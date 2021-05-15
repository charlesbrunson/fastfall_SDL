#include "fastfall/game/phys/collision/Contact.hpp"

namespace ff {

const char* contactTypeToString(ContactType t) {
	const char* types[] = {
		"No solution",
		"Single",
		"Wedge same",
		"Wedge opposite",
		"Wedge wall",
		"Crush horizontal",
		"Crush vertical"
	};
	return types[static_cast<unsigned char>(t)];
};

}