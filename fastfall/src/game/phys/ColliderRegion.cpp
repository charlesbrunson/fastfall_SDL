#include "fastfall/game/phys/ColliderRegion.hpp"

#include "fastfall/game/World.hpp"

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

void imgui_component(World& w, ID<ColliderRegion> id) {
    auto& col = w.at(id);
    ImGui::Text("Curr Position:  %3.2f, %3.2f",     col.getPosition().x, col.getPosition().y);
    ImGui::Text("Prev Position:  %3.2f, %3.2f", col.getPrevPosition().x, col.getPrevPosition().y);
    ImGui::Text("Delta Position: %3.2f, %3.2f", col.getDeltaPosition().x, col.getDeltaPosition().y);
    ImGui::Text("Velocity:       %3.2f, %3.2f", col.velocity.x, col.velocity.y);
    ImGui::Text("Delta Vel:      %3.2f, %3.2f", col.delta_velocity.x, col.delta_velocity.y);

    float pos[2] = { col.getPosition().x, col.getPosition().y };

    if (ImGui::DragFloat2("Set Pos", pos)) {
        col.setPosition(Vec2f(pos[0], pos[1]));

        Vec2f nVel = (col.getPosition() - col.getPrevPosition()) / (1.0 / 60.0);
        col.delta_velocity = nVel - col.velocity;
        col.velocity = nVel;
    }
}

ColliderRegion::QuadIterator& ColliderRegion::QuadIterator::operator++() {
    curr_quad = region->next_quad_in_rect(area, *curr_quad, tile_area);
    return *this;
}

const ColliderRegion::QuadIterator::value_type* ColliderRegion::QuadIterator::operator->() const { return region->get_quad(*curr_quad); }
const ColliderRegion::QuadIterator::value_type& ColliderRegion::QuadIterator::operator* () const { return *region->get_quad(*curr_quad); }

ColliderRegion::QuadIterator ColliderRegion::QuadArea::begin() const {
    Recti tile_area;
    auto quad = region->first_quad_in_rect(area, tile_area);
    return QuadIterator{ region, area, quad, tile_area };
}
ColliderRegion::QuadIterator ColliderRegion::QuadArea::end()   const { return QuadIterator{ region, area, std::nullopt, {} }; }

}