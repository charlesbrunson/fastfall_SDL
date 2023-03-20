#pragma once

#include "fastfall/game/systems/CameraSystem.hpp"

namespace ff {
    class World;
}

class SimpleCamTarget : public ff::CameraTarget {
public:
	SimpleCamTarget(ff::CamTargetPriority priority, std::function<ff::Vec2f(ff::World&)>&& callback);

	void update(ff::World& w, secs delta) override;

	std::function<ff::Vec2f(ff::World&)> pos_callback;

};

