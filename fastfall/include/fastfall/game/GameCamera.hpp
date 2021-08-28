#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"

#include <array>

namespace ff {

class GameCamera {
public:

	using TargetID = unsigned int;

	enum class TargetPriority : unsigned {
		LOW = 0u,
		MEDIUM = 1u,
		HIGH = 2u
	};
	enum class TargetBehavior {
		STANDARD
	};
	enum class TargetType {
		NONE,
		MOVING,
		STATIC
	};

	struct Target {
		const Vec2f* movingTarget;
		Vec2f staticTarget;
		TargetType type = TargetType::NONE;

		Vec2f offset = Vec2f(0, 0);
		Vec2f offsetPrev = Vec2f(0, 0);
		TargetPriority priority = TargetPriority::LOW;
		TargetBehavior behavior = TargetBehavior::STANDARD;
	};

	GameCamera(Vec2f initPos = Vec2f(0, 0));

	void update(secs deltaTime);

	//TargetID addTarget(Target target);
	void addTarget(Target target);
	void removeTarget(TargetPriority priority);
	void removeAllTargets();

	const std::array<Target, 3>& getTargets() const {
		return targets;
	}

	float zoomFactor = 1.f;
	bool lockPosition = false;
	Vec2f currentPosition;

private:
	//TargetID targetCounter = 0u;

	std::array<Target, 3> targets;
	static constexpr size_t TARGET_COUNT = 3u;


};

}