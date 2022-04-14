#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/GameContext.hpp"

#include <array>

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
	CameraTarget(GameContext context, CamTargetPriority priority);
	virtual ~CameraTarget();

	virtual void update(secs delta) = 0;

	virtual Vec2f get_target_pos() const = 0;

	CamTargetPriority get_priority() const { return m_priority; };
	CamTargetState get_state() const { return m_state; };

	friend bool operator< (const CameraTarget& lhs, const CameraTarget& rhs) {
		return lhs.m_priority < rhs.m_priority;
	}

private:
	friend class GameCamera;

	bool has_camera = false;
	CamTargetState m_state = CamTargetState::Active;
	CamTargetPriority m_priority;
	GameContext m_context;
};

class GameCamera {
public:

	GameCamera(Vec2f initPos = Vec2f(0, 0));

	void update(secs deltaTime);

	void addTarget(CameraTarget& target);
	bool removeTarget(CameraTarget& target);

	void removeAllTargets();

	const std::vector<CameraTarget*>& getTargets() const;

	Vec2f getPosition(float interpolation);

	float zoomFactor = 1.f;
	bool lockPosition = false;

	Vec2f deltaPosition;
	Vec2f prevPosition;
	Vec2f currentPosition;

private:

	CameraTarget* active_target = nullptr;
	std::vector<CameraTarget*> targets;
};




}