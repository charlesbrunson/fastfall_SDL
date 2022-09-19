#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"

namespace ff {

struct CollisionContext {
    ColliderRegion *collider;
    Collidable *collidable;
};

}
