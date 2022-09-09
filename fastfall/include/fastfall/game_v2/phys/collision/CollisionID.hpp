#pragma once

#include "fastfall/game_v2/phys/collider_coretypes/ColliderQuadID.hpp"
#include "fastfall/util/id.hpp"

namespace ff {

class Collidable;
class ColliderRegion;

struct CollisionID {
    ID<Collidable> collidable;
    ID<ColliderRegion> collider;
    QuadID quad;
};

}