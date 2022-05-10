#include "fastfall/game/phys/collision/Contact.hpp"

namespace ff {

std::string_view contactTypeToString(ContactType t) {
	static const std::string_view types[] = {
		"No solution",
		"Single",
		"Wedge",
		"Crush horizontal",
		"Crush vertical"
	};
	return types[static_cast<int>(t)];
};

}