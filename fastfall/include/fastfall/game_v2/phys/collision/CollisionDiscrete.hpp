#pragma once

#include "fastfall/game_v2/phys/collider_coretypes/ColliderQuad.hpp"
#include "fastfall/game_v2/phys/Collidable.hpp"
#include "fastfall/game_v2/phys/ColliderRegion.hpp"
#include "fastfall/game_v2/phys/collision/CollisionID.hpp"
#include "fastfall/game_v2/phys/collision/CollisionContext.hpp"

#include "CollisionAxis.hpp"

#include <assert.h>
#include <array>

namespace ff {

//class Arbiter;

class CollisionDiscrete {
public:

    enum class Type {
        PrevFrame,
        CurrFrame
    };

    CollisionDiscrete(CollisionID t_id, Type collidePrev);

	inline void setPrevious() { collision_time = Type::PrevFrame; };
	inline const Contact& getContact() const noexcept { return contact; };

	void updateContact() noexcept;

	void setAxisApplied(Vec2f ortho_normal) noexcept {
		for (auto& axis : axes) {
			if (axis.contact.ortho_n == ortho_normal)
				axis.applied = true;
		}
	}

	inline const CollisionAxis& getCollisionAxis(unsigned ndx) { 
		assert(ndx < axis_count);
		return axes.at(ndx); 
	};

	inline unsigned getAxisCount() { return axis_count; };

	void reset(CollisionContext ctx, ColliderQuad quad, Type collidePrev);

    CollisionID id;

	int getChosenAxis() const { return chosen_axis; };

protected:

	static constexpr float VALLEY_FLATTEN_THRESH = 0.25f;

	void initCollidableData();
    /*
    {
		if (!collidePrevious) {
			cBox  = cAble->getBox();
			cPrev = cAble->getPrevBox();
		}
		else {
			cBox  = cAble->getPrevBox();
			cPrev = cAble->getPrevBox();
		}
		cMid = math::rect_mid(cBox);
		cHalf = cBox.getSize() / 2.f;
	}
    */

	void createAxes() noexcept;
	void evalContact() noexcept;

	CollisionAxis createFloor(const AxisPreStep& initData) noexcept;
	CollisionAxis createCeil(const AxisPreStep& initData) noexcept;
	CollisionAxis createEastWall(const AxisPreStep& initData) noexcept;
	CollisionAxis createWestWall(const AxisPreStep& initData) noexcept;


	ColliderQuad cQuad;

	Contact contact;
	int chosen_axis = -1;

	Type collision_time;

	// some derived info
	Vec2f tPos;
	Rectf tArea;
	Vec2f tMid;
	Vec2f tHalf;

	Rectf cBox;
	Rectf cPrev;
	Vec2f cMid;
	Vec2f cHalf;

	ordinal_array<bool> valleys = { false, false, false, false };

	std::array<CollisionAxis, 5> axes; // 5 axes in worst case
	unsigned axis_count = 0;

	//Arbiter* arbiter = nullptr;
};

}
