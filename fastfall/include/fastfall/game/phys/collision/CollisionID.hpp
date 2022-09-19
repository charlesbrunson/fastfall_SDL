#pragma once

#include "fastfall/game/phys/collider_coretypes/ColliderQuadID.hpp"
#include "fastfall/util/id.hpp"

namespace ff {

class Collidable;
class ColliderRegion;

struct CollisionID {
    ID<Collidable> collidable;
    ID<ColliderRegion> collider;
    QuadID quad;

    bool operator< (const CollisionID& other) const
    {
        return collidable < other.collidable
            || (collidable == other.collidable && collider < other.collider)
            || (collidable == other.collidable && collider == other.collider && quad < other.quad);
    }
};

}