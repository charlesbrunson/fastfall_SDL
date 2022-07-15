#pragma once

#include "fastfall/game/ID.hpp"

#include <vector>

namespace ff {

struct Entity {
	ID<Entity> m_id;
	std::vector<GenericID> components;
};

}