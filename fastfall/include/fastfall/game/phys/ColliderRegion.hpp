#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

#include <assert.h>
#include <math.h>

namespace ff {

class ColliderRegion {
public:
	explicit ColliderRegion(Vec2i initialPosition = Vec2i(0, 0));
	virtual ~ColliderRegion() = default;

	virtual void get_quads_in_rect(Rectf area, std::vector<std::pair<Rectf, QuadID>>& out_buffer) const = 0;

	virtual void update(secs deltaTime) = 0;

	virtual const ColliderQuad* get_quad(QuadID quad_id) const noexcept = 0;

	const ColliderSurface* get_surface_collider(ColliderSurfaceID id) const noexcept;
	const SurfaceMaterial* get_surface_material(ColliderSurfaceID id) const noexcept;

	Vec2f getPrevPosition() const noexcept;
	Vec2f getPosition() const noexcept;

	bool hasMoved() const noexcept;
	Vec2f getDeltaPosition() const noexcept;

	void teleport(Vec2f pos);
	void setPosition(Vec2f pos, bool updatePrev = true);

	Rectf getBoundingBox() const noexcept;
	Rectf getSweptBoundingBox() const noexcept;

	Vec2f velocity;
	Vec2f delta_velocity;

	virtual bool on_precontact(World& w, const ContinuousContact& contact, secs duration) const { return true; };
	virtual void on_postcontact(World& w, const AppliedContact& contact) const {};

protected:
	Rectf boundingBox;
	Rectf prevBoundingBox;

private:
	Vec2f prevPosition;
	Vec2f position;
};

template<typename T>
concept ColliderType = std::is_base_of_v<ColliderRegion, T>;

}
