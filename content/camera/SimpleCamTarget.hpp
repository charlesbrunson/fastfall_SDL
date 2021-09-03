#pragma once

#include "fastfall/game/GameCamera.hpp"

//namespace ff {

class SimpleCamTarget : public ff::CameraTarget {
public:
	SimpleCamTarget(ff::GameContext context, ff::CamTargetPriority priority, const ff::Vec2f* pos, ff::Vec2f offset, bool add_to_cam = true);
	SimpleCamTarget(ff::GameContext context, ff::CamTargetPriority priority, const ff::Vec2f* pos, bool add_to_cam = true);

	void update(secs delta) override;
	ff::Vec2f get_target_pos() const override;

	const ff::Vec2f* position_ptr;
	ff::Vec2f m_offset;

};

