#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/game_v2/phys/Collidable.hpp"
#include "fastfall/game_v2/phys/collider_coretypes/ColliderQuad.hpp"

#include <assert.h>
#include <math.h>

namespace ff {

class ColliderRegion /*: public sf::Drawable*/ {
public:

	ColliderRegion(Vec2i initialPosition = Vec2i(0, 0)) :
		position(initialPosition),
		velocity(0.f, 0.f),
		prevPosition(initialPosition)
	{
		//id = ColliderID{ colliderIDCounter++ };
		//assert(id.value != ColliderID::NO_ID);
	}
	virtual ~ColliderRegion() {};

	virtual void get_quads_in_rect(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& out_buffer) const = 0;

	virtual void update(secs deltaTime) = 0;

	virtual const ColliderQuad* get_quad(QuadID quad_id) const noexcept = 0;

	const ColliderSurface* get_surface_collider(ColliderSurfaceID id) const noexcept
	{
		if (auto* q = get_quad(id.quad_id); 
			q && q->surfaces[id.dir].hasSurface)
		{
			return &q->surfaces[id.dir].collider;
		}
		return nullptr;
	}

	const SurfaceMaterial* get_surface_material(ColliderSurfaceID id) const noexcept
	{
		if (auto* q = get_quad(id.quad_id); 
			q && q->surfaces[id.dir].hasSurface)
		{
			return &q->surfaces[id.dir].material;
		}
		return nullptr;
	}

	inline Vec2f getPrevPosition() const noexcept { return prevPosition; };
	inline Vec2f getPosition() const noexcept { return position; };

	inline bool hasMoved() const noexcept { return getPosition() != getPrevPosition(); }
	inline Vec2f getDeltaPosition() const noexcept { return getPosition() - getPrevPosition(); }

	inline void teleport(Vec2f pos) {
		prevPosition = pos;
		position = pos;
	}

	inline void setPosition(Vec2f pos, bool updatePrev = true) {
		if (updatePrev)
			prevPosition = position;

		position = pos;
	}

	inline Rectf getBoundingBox() const noexcept {
		return Rectf(Vec2f(boundingBox.getPosition()) + position, Vec2f(boundingBox.getSize()));
	}

	
	inline Rectf getSweptBoundingBox() const noexcept {
		Rectf prevB( Vec2f(prevBoundingBox.getPosition()) + prevPosition, Vec2f(prevBoundingBox.getSize()) );
		Rectf currB( Vec2f(boundingBox.getPosition()) + position, Vec2f(boundingBox.getSize()) );

		return math::rect_bound(prevB, currB);
	}

	Vec2f velocity;
	Vec2f delta_velocity;

	//Collidable* attached = nullptr;

	virtual bool on_precontact(QuadID quad_id, const Contact& contact, secs duration) const { return true; };
	virtual void on_postcontact(const PersistantContact& contact) const {};

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
