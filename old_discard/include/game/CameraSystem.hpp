#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "GameContext.hpp"
#include "camera/CameraTarget.hpp"

#include "fastfall/util/id_map.hpp"

#include <array>
#include <optional>

namespace ff {

class CameraSystem {
public:

	CameraSystem();
	CameraSystem(Vec2f initPos);

	CameraSystem(const CameraSystem&);
	CameraSystem& operator=(const CameraSystem&);

	void update(secs deltaTime);


	Vec2f getPosition(float interpolation);

	float zoomFactor = 1.f;
	bool lockPosition = false;

	Vec2f deltaPosition;
	Vec2f prevPosition;
	Vec2f currentPosition;

	poly_id_map<CameraTarget> targets;

	CameraTarget* get_active_target() { return active_target ? &targets.at(*active_target) : nullptr; }
	const CameraTarget* get_active_target() const { return active_target ? &targets.at(*active_target) : nullptr; }

private:

	void set_component_callbacks();
	void add_to_ordered(ID<CameraTarget> id);

	std::optional<ID<CameraTarget>> active_target;
	std::vector<ID<CameraTarget>> ordered_targets;
};

}