#pragma once

#include "fastfall/game/GameCamera.hpp"

namespace ff {

class SimpleCamTarget : public CameraTarget {
public:
	SimpleCamTarget(GameContext context, CamTargetPriority priority, const Vec2f* pos, Vec2f offset, bool add_to_cam = true)
		: CameraTarget(context, priority, add_to_cam)
		, position_ptr(pos)
		, m_offset(offset)
	{
	}

	SimpleCamTarget(GameContext context, CamTargetPriority priority, const Vec2f* pos, bool add_to_cam = true)
		: CameraTarget(context, priority, add_to_cam)
		, position_ptr(pos)
	{
	}

	void update(secs delta) override 
	{
	}

	Vec2f get_target_pos() const override {
		return *position_ptr + m_offset;
	}

	const Vec2f* position_ptr;
	Vec2f m_offset;

};

}