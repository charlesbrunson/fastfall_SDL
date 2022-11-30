#pragma once

#include "fastfall/game/ComponentID.hpp"

#include <set>

namespace ff {

struct Entity {
    std::set<ComponentID> components;
};

}