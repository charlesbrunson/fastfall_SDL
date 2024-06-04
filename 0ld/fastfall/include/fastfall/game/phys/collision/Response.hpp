#pragma once

#include "Contact.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/Collidable.hpp"

namespace ff {

namespace phys_resp {

	enum class type {
		STANDARD,
		FLATTEN
	};

	Vec2f get(const Collidable& body, const AppliedContact& contact, phys_resp::type response_type = phys_resp::type::STANDARD);

}

}
