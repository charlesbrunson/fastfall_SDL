#pragma once

#include "fastfall/game_v2/phys/Collidable.hpp"
#include "fastfall/game_v2/phys/ColliderRegion.hpp"

namespace ff {

struct CollisionContext {
    ColliderRegion *collider;
    Collidable *collidable;
};

}
