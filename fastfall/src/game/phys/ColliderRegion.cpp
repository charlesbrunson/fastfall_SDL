#include "fastfall/game/phys/ColliderRegion.hpp"

#include "imgui.h"

namespace ff {

ColliderRegion::ColliderRegion(Vec2i initialPosition)
    : position(initialPosition)
    , velocity(0.f, 0.f)
    , prevPosition(initialPosition)
{
}

const ColliderSurface* ColliderRegion::get_surface_collider(ColliderSurfaceID id) const noexcept
{
    if (auto* q = get_quad(id.quad_id);
            q && q->surfaces[id.dir].hasSurface)
    {
        return &q->surfaces[id.dir].collider;
    }
    return nullptr;
}

const ColliderSurface* ColliderRegion::get_surface_collider(std::optional<ColliderSurfaceID> id) const noexcept {
    return id ? get_surface_collider(*id) : nullptr;
}

const SurfaceMaterial* ColliderRegion::get_surface_material(ColliderSurfaceID id) const noexcept
{
    if (auto* q = get_quad(id.quad_id);
            q && q->surfaces[id.dir].hasSurface)
    {
        return &q->surfaces[id.dir].material;
    }
    return nullptr;
}

Vec2f ColliderRegion::getPrevPosition() const noexcept {
    return prevPosition;
}

Vec2f ColliderRegion::getPosition() const noexcept {
    return position;
}

bool ColliderRegion::hasMoved() const noexcept {
    return getPosition() != getPrevPosition();
}

Vec2f ColliderRegion::getDeltaPosition() const noexcept {
    return getPosition() - getPrevPosition();
}

void ColliderRegion::teleport(Vec2f pos) {
    prevPosition = pos;
    position = pos;
}

void ColliderRegion::setPosition(Vec2f pos, bool updatePrev) {
    if (updatePrev)
        prevPosition = position;

    position = pos;
}

Rectf ColliderRegion::getBoundingBox() const noexcept {
    return Rectf(Vec2f(boundingBox.getPosition()) + position, Vec2f(boundingBox.getSize()));
}

Rectf ColliderRegion::getSweptBoundingBox() const noexcept {
    Rectf prevB( Vec2f(prevBoundingBox.getPosition()) + prevPosition, Vec2f(prevBoundingBox.getSize()) );
    Rectf currB( Vec2f(boundingBox.getPosition()) + position, Vec2f(boundingBox.getSize()) );

    return math::rect_bound(prevB, currB);
}

}