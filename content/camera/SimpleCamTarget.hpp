#pragma once

#include "fastfall/game/CameraSystem.hpp"

class SimpleCamTarget : public ff::CameraTarget {
public:
	SimpleCamTarget(ff::GameContext context, ff::CamTargetPriority priority, std::function<ff::Vec2f()>&& callback);

	void update(secs delta) override;
	ff::Vec2f get_target_pos() const override;

	std::function<ff::Vec2f()> pos_callback;

};

