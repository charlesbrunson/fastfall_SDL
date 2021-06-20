#pragma once

#include "fastfall/util/math.hpp"
//#include "util/Updatable.hpp"

#include "fastfall/game/phys/Collidable.hpp"
//#include "ColliderSurface.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"

#include <assert.h>
#include <math.h>

#include "fastfall/game/phys/Identity.hpp"

namespace ff {

class ColliderRegion /*: public sf::Drawable*/ {
public:

	ColliderRegion(Vec2i initialPosition = Vec2i(0, 0)) :
		position(initialPosition),
		velocity(0.f, 0.f),
		prevPosition(initialPosition)
	{
		static unsigned colliderIDCounter = ColliderID::NO_ID + 1u;

		id = ColliderID{ colliderIDCounter++ };
		assert(id.value != ColliderID::NO_ID);
	}

	virtual void getQuads(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& buffer) const = 0;

	virtual void update(secs deltaTime) = 0;

	virtual const ColliderQuad* get_quad(int quad_id) const noexcept = 0;

	inline Vec2f getPrevPosition() const noexcept { return prevPosition; };
	inline Vec2f getPosition() const noexcept { return position; };

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

	inline ColliderID get_ID() const noexcept { return id; };

	Vec2f velocity;
	Vec2f delta_velocity;

	Collidable* attached = nullptr;

protected:
	Rectf boundingBox;

	ColliderID id;

private:

	Vec2f prevPosition;
	Vec2f position;

};

}