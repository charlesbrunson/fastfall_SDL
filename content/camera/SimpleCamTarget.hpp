#pragma once

#include "fastfall/game/GameCamera.hpp"

//namespace ff {

class SimpleCamTarget : public ff::CameraTarget {
public:
	SimpleCamTarget(ff::GameContext context, ff::CamTargetPriority priority, std::function<ff::Vec2f()>&& callback);

	void update(secs delta) override;
	ff::Vec2f get_target_pos() const override;

	//const ff::Vec2f* position_ptr;

	std::function<ff::Vec2f()> pos_callback;
	//ff::Vec2f m_offset;

};

