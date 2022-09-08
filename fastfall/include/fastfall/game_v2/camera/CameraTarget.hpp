#pragma once

#include "fastfall/util/math.hpp"
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


class CameraTarget {
public:
	CameraTarget(CamTargetPriority priority);
	virtual ~CameraTarget() = default;

	virtual void update(secs delta) = 0;

	virtual Vec2f get_target_pos() const = 0;

	CamTargetPriority get_priority() const { return m_priority; };
	CamTargetState get_state() const { return m_state; };

	friend bool operator< (const CameraTarget& lhs, const CameraTarget& rhs) {
		return lhs.m_priority < rhs.m_priority;
	}

private:
	friend class CameraSystem;

	bool has_camera = false;
	CamTargetState m_state = CamTargetState::Active;
	CamTargetPriority m_priority;
};

}
