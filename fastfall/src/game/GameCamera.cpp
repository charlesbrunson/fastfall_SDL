#include "fastfall/game/GameCamera.hpp"


namespace ff {

GameCamera::GameCamera(Vec2f initPos) :
	currentPosition(initPos)
{

};

void GameCamera::update(secs deltaTime) {

	if (deltaTime > 0.0 && !lockPosition) {

		for (int i = 2; i >= 0; i--) {
			if (targets[i].type != TargetType::NONE) {
				if (targets[i].type == TargetType::MOVING) {
					currentPosition = *targets[i].movingTarget + targets[i].offset;
				}
				else if (targets[i].type == TargetType::STATIC) {
					currentPosition = targets[i].staticTarget + targets[i].offset;
				}
				break;
			}

			targets[i].offsetPrev = targets[i].offset;
		}

	}
}

void GameCamera::addTarget(GameCamera::Target target) {
	targets[static_cast<size_t>(target.priority)] = target;
}
void GameCamera::removeTarget(TargetPriority priority) {
	targets[static_cast<size_t>(priority)].type = TargetType::NONE;
}

void GameCamera::removeAllTargets() {
	for (size_t i = 0; i < TARGET_COUNT; i++) {
		targets[i].type = TargetType::NONE;
	}
}

}