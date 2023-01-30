#pragma once

#include "fastfall/game/ComponentID.hpp"
#include "fastfall/game/Actor.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"

#include <set>

namespace ff {

struct Entity {
    copyable_unique_ptr<Actor> actor;
    std::set<ComponentID> components;
};

}