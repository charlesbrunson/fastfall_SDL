#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/camera/CameraTarget.hpp"

#include "fastfall/util/id_map.hpp"

#include <array>
#include <optional>


namespace ff {

class World;

class CameraSystem {
public:

	CameraSystem() = default;
	CameraSystem(Vec2f initPos);

	inline void set_world(World* w) { world = w; }
	void notify_created(ID<CameraTarget> id);
	void notify_erased(ID<CameraTarget> id);

	void update(secs deltaTime);

	Vec2f getPosition(float interpolation);

	float zoomFactor = 1.f;
	bool lockPosition = false;

	Vec2f deltaPosition;
	Vec2f prevPosition;
	Vec2f currentPosition;

private:
	void add_to_ordered(ID<CameraTarget> id);

	std::optional<ID<CameraTarget>> active_target;
	std::vector<ID<CameraTarget>> ordered_targets;
	World* world = nullptr;
};

}
