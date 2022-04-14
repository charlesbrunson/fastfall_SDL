#pragma once

//#include "Collision.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"

#include "CollisionAxis.hpp"

#include <assert.h>
#include <array>

namespace ff {

class CollisionDiscrete {
public:
	CollisionDiscrete(const Collidable* collidable, const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion, bool collidePreviousFrame = false);

	inline void setPrevious() { collidePrevious = true; };
	inline Contact getContact() const noexcept { return contact; };
	inline const Contact* getContactPtr() const noexcept { return &contact; };

	void updateContact() noexcept;

	void setAxisApplied(Vec2f ortho_normal) noexcept {
		for (auto& axis : axes) {
			if (axis.contact.ortho_normal == ortho_normal)
				axis.applied = true;
		}
	}

	inline const CollisionAxis& getCollisionAxis(unsigned ndx) { 
		assert(ndx < axis_count);
		return axes.at(ndx); 
	};

	inline unsigned getAxisCount() {
		return axis_count; 
	};

	void reset(const ColliderQuad* collisionTile, const ColliderRegion* colliderRegion, bool collidePreviousFrame);

	inline bool tileValid() const noexcept {
		return cTile && cTile->hasAnySurface();
	};

protected:

	static constexpr float VALLEY_FLATTEN_THRESH = 0.25f;

	void initCollidableData() {
		if (!collidePrevious) {
			cBox = cAble->getBox();
			cPrev = cAble->getPrevBox();
		}
		else {
			cBox = cAble->getPrevBox();
		}
		cMid = math::rect_mid(cBox);
		cHalf = cBox.getSize() / 2.f;
	}

	void createAxes() noexcept;
	void evalContact() noexcept;

	CollisionAxis createFloor(const AxisPreStep& initData) noexcept;
	CollisionAxis createCeil(const AxisPreStep& initData) noexcept;
	CollisionAxis createEastWall(const AxisPreStep& initData) noexcept;
	CollisionAxis createWestWall(const AxisPreStep& initData) noexcept;

	const ColliderRegion* region;
	const ColliderQuad* cTile;
	const Collidable* cAble;

	ColliderQuad cQuad;

	Contact contact;

	bool collidePrevious;

	// some derived info
	Vec2f tPos;
	Rectf tArea;
	Vec2f tMid;
	Vec2f tHalf;

	Rectf cBox;
	Rectf cPrev;
	Vec2f cMid;
	Vec2f cHalf;

	bool valley_NE = false;
	bool valley_SE = false;

	bool valley_NW = false;
	bool valley_SW = false;

	std::array<CollisionAxis, 5> axes; // 5 axes in worst case
	unsigned axis_count = 0;

	inline float getYforX(const Linef& onLine, float X) {
		Vec2f v = math::vector(onLine);
		assert(v.x != 0.f); // no vertical lines
		float scale = ((X - onLine.p1.x) / v.x);
		return (scale * v.y) + onLine.p1.y;
	};
};

}
