#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/GameContext.hpp"

#include "fastfall/util/slot_map.hpp"

#include <array>
#include <optional>

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
	virtual ~CameraTarget();

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
	//GameContext m_context;
};

struct camtarget_id {
	slot_key value;
	bool operator==(const camtarget_id& other) const { return value == other.value; };
	bool operator!=(const camtarget_id& other) const { return value != other.value; };
};

class CameraSystem {
public:

	CameraSystem() = default;
	CameraSystem(Vec2f initPos);

	void update(secs deltaTime);

	template<class T, class... Args>
	camtarget_id create(Args&&... args) 
	{
		auto id = camtarget_id{
			target_slots.emplace_back()
		};

		target_slots.at(id.value) = std::make_unique<T>(std::forward<Args>(args)...);

		add_to_ordered(id);
		return id;
	}

	bool erase(camtarget_id target);


	CameraTarget* get(camtarget_id target) {
		return target_slots.exists(target.value) ? target_slots.at(target.value).get() : nullptr;
	}

	const CameraTarget* get(camtarget_id target) const {
		return target_slots.exists(target.value) ? target_slots.at(target.value).get() : nullptr;
	}

	bool exists(camtarget_id target) const {
		return target_slots.exists(target.value);
	}

	const std::vector<camtarget_id>& getTargets() const;

	Vec2f getPosition(float interpolation);

	float zoomFactor = 1.f;
	bool lockPosition = false;

	Vec2f deltaPosition;
	Vec2f prevPosition;
	Vec2f currentPosition;

private:
	void add_to_ordered(camtarget_id id);

	std::optional<camtarget_id> active_target;

	slot_map<std::unique_ptr<CameraTarget>> target_slots;
	std::vector<camtarget_id> ordered_targets;
};




}