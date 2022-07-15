#pragma once

#include "fastfall/util/math.hpp"
//#include "util/Updatable.hpp"

#include "fastfall/game/phys/Collidable.hpp"
//#include "ColliderSurface.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

#include <assert.h>
#include <math.h>

//#include "fastfall/game/phys/Identity.hpp"

namespace ff {

class ColliderRegion /*: public sf::Drawable*/ {
public:

	ColliderRegion(Vec2i initialPosition = Vec2i(0, 0)) :
		position(initialPosition),
		velocity(0.f, 0.f),
		prevPosition(initialPosition)
	{
		//static unsigned colliderIDCounter = ColliderID::NO_ID + 1u;

		//id = ColliderID{ colliderIDCounter++ };
		//assert(id.value != ColliderID::NO_ID);
	}

	virtual ~ColliderRegion() {};

	virtual void getQuads(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& buffer) const = 0;

	virtual void update(secs deltaTime) = 0;


	virtual const ColliderQuad* get_quad(QuadID quad_id) const noexcept = 0;

	const ColliderSurface* get_surface(ColliderSurfaceID id) const {
		if (auto quad = get_quad(id.quad))
		{
			return quad->getSurface(id.dir);
		}
		return nullptr;
	}

	const ColliderSurface* get_surface(std::optional<ColliderSurfaceID> id_opt) const {
		return id_opt ? get_surface(*id_opt) : nullptr;
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
concept ColliderType = std::derived_from<T, ColliderRegion>;

}
