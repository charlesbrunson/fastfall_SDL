#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/game_v2/phys/collider_coretypes/ColliderSurface.hpp"
#include "fastfall/game_v2/tile/Tile.hpp"
#include "CollisionID.hpp"

#include <string>

namespace ff {

class Arbiter;

enum class ContactType : unsigned char {
    NO_SOLUTION,

    // typical case
    // from a single arbiter
    SINGLE,

    // wedge solution
    // derived from two arbiters
    WEDGE,

    // crush solution
    // derived from two arbiters
    CRUSH_HORIZONTAL,
    CRUSH_VERTICAL
};

std::string_view contactTypeToString(ContactType t);

struct DiscreteContact {
    ColliderSurface collider;

    Vec2f ortho_n;
    Vec2f collider_n;

    float separation = 0.f;
    Vec2f position;

    // offset to stick to the surface after the contact
    // multiply with ortho_normal
    float stickOffset = 0.f;
    Linef stickLine;

    bool hasContact = false;
    bool hasValley = false;

    const SurfaceMaterial *material = nullptr;
    std::optional<CollisionID> id;

    bool is_resolvable() const noexcept {
        return ortho_n != Vec2f();
    }

    Vec2f surface_vel() const {
        return (material ? collider_n.righthand() * material->velocity : Vec2f{});
    }
};

struct ContinuousContact : public DiscreteContact {

    ContinuousContact() = default;
    ContinuousContact(const DiscreteContact& other)
            : DiscreteContact(other)
    {}

    // moment that the object started intersecting the collider,
    // represented as a fraction of the tick deltatime [0, 1.0]
    // used by continuous collision
    float impactTime = -1.0;
    bool hasImpactTime = false;

    bool isSlip = false;

    bool quad_valid = false;

    // the velocity of the surface in contact
    // relative to worldspace
    Vec2f velocity;

    secs touchDuration = 0.0;

    bool is_transposed = false;

    bool transposable() const noexcept {
        return !is_transposed
               && math::is_vertical(ortho_n)
               && std::abs(collider_n.x) > std::abs(collider_n.y)
               && hasImpactTime
               && !hasValley;
    }

    void transpose() noexcept {
        if (!is_transposed) {
            Vec2f alt_ortho_normal = (collider_n.x < 0.f ? Vec2f(-1.f, 0.f) : Vec2f(1.f, 0.f));
            float alt_separation = abs((collider_n.y * separation) / collider_n.x);
            ortho_n = alt_ortho_normal;
            separation = alt_separation;
            is_transposed = true;
        }
    }

    bool operator<(const ContinuousContact &other) const;
};

struct AppliedContact : public ContinuousContact {
    AppliedContact() = default;
    AppliedContact(const ContinuousContact& other)
        : ContinuousContact(other)
    {}

    ContactType type = ContactType::NO_SOLUTION;
};

}