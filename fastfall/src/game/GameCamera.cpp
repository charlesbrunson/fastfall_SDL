#include "fastfall/game/GameCamera.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/engine/config.hpp"

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


	if (debug_draw::hasTypeEnabled(debug_draw::Type::CAMERA_VISIBLE) && !debug_draw::repeat((void*)this, currentPosition)) {


		debug_draw::set_offset(currentPosition);

		Rectf area{
			Vec2f{GAME_W_F * -0.5f, GAME_H_F * -0.5f},
			Vec2f{GAME_W_F, GAME_H_F}
		};

		auto& visible_box = createDebugDrawable<VertexArray, debug_draw::Type::CAMERA_VISIBLE>((const void*)this, Primitive::LINE_LOOP, 4);

		for (int i = 0; i < visible_box.size(); i++) {
			visible_box[i].color = Color::White;
		}
		visible_box[0].pos = math::rect_topleft(area);
		visible_box[1].pos = math::rect_topright(area);
		visible_box[2].pos = math::rect_botright(area);
		visible_box[3].pos = math::rect_botleft(area);

		debug_draw::set_offset();
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