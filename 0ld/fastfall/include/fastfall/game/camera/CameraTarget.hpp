#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/engine/time/time.hpp"

namespace ff {

enum class CamTargetPriority {
	Low = 0,
	Medium = 1,
	High = 2
};

enum class CamTargetState {
	Inactive,
	Active
};

class World;

class CameraTarget {
public:
	CameraTarget(CamTargetPriority priority);
	virtual ~CameraTarget() = default;

	virtual void update(World& w, secs delta) = 0;

	Vec2f               get_target_pos() const;
	CamTargetPriority   get_priority() const;
	CamTargetState      get_state() const;

	friend bool operator< (const CameraTarget& lhs, const CameraTarget& rhs) {
		return lhs.m_priority < rhs.m_priority;
	}

protected:
    Vec2f position;

private:
	bool has_camera = false;
	CamTargetState m_state = CamTargetState::Active;
	CamTargetPriority m_priority;

    friend class CameraSystem;
};

void imgui_component(World& w, ID<CameraTarget> id);

}
