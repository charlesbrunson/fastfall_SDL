#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/GameContext.hpp"
#include "fastfall/game/camera/CameraTarget.hpp"

#include "fastfall/util/slot_map.hpp"
#include "fastfall/game/ComponentList.hpp"

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

	ComponentList<CameraTarget, true> targets;

	bool has_active_target() const { return m_active_target.has_value(); }
	CameraTarget* active_target() { return m_active_target ? &targets.at(*m_active_target) : nullptr; }
	const CameraTarget* active_target() const { return m_active_target ? &targets.at(*m_active_target) : nullptr; }

private:

	void set_component_callbacks();
	void add_to_ordered(ID<CameraTarget> id);

	std::optional<ID<CameraTarget>> m_active_target;
	std::vector<ID<CameraTarget>> m_ordered_targets;
};




}