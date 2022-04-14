#include "fastfall/game/GameCamera.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/engine/config.hpp"

#include "fastfall/game/InstanceInterface.hpp"

namespace ff {


CameraTarget::CameraTarget(GameContext context, CamTargetPriority priority)
	: m_context(context)
	, m_priority(priority)
{
}

CameraTarget::~CameraTarget() {
	if (has_camera) {
		instance::cam_remove_target(m_context, *this);
	}
}

GameCamera::GameCamera(Vec2f initPos) :
	currentPosition(initPos)
{

};

void GameCamera::update(secs deltaTime) {

	if (deltaTime > 0.0) {

		prevPosition = currentPosition;
		deltaPosition = Vec2f{};

		for (auto target : targets) {
			target->update(deltaTime);
		}

		if (active_target && active_target != targets.back()) {
			active_target->m_state = CamTargetState::Inactive;
			targets.back()->m_state = CamTargetState::Active;
			active_target = targets.back();
		}
		else if (!active_target && targets.size() > 0) {
			targets.back()->m_state = CamTargetState::Active;
			active_target = targets.back();
		}
		else if (targets.empty()) {
			active_target = nullptr;
		}

		if (active_target && !lockPosition) {
			auto pos = active_target->get_target_pos();
			deltaPosition = pos - currentPosition;
			currentPosition = pos;
		}
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::CAMERA_VISIBLE) && !debug_draw::repeat((void*)this, currentPosition)) {

		debug_draw::set_offset(currentPosition);

		constexpr Rectf area{
			Vec2f{GAME_W_F * -0.5f, GAME_H_F * -0.5f},
			Vec2f{GAME_W_F, GAME_H_F}
		};

		auto& visible_box = createDebugDrawable<VertexArray, debug_draw::Type::CAMERA_VISIBLE>(
			(const void*)this, Primitive::LINE_LOOP, 4);

		for (int i = 0; i < visible_box.size(); i++) {
			visible_box[i].color = Color::White;
		}
		visible_box[0].pos = math::rect_topleft(area);
		visible_box[1].pos = math::rect_topright(area);
		visible_box[2].pos = math::rect_botright(area);
		visible_box[3].pos = math::rect_botleft(area);

		debug_draw::set_offset();
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::CAMERA_TARGET)) {

		if (!debug_draw::repeat((void*)this, currentPosition)) {

			debug_draw::set_offset(currentPosition);

			auto& current_crosshair = createDebugDrawable<VertexArray, debug_draw::Type::CAMERA_TARGET>(
				(const void*)this, Primitive::LINE_LOOP, 4);

			//LOG_INFO("{}", (void*)this);
			for (int i = 0; i < current_crosshair.size(); i++) {
				current_crosshair[i].color = Color::White;
			}
			current_crosshair[0].pos = Vec2f{ -2.f,  0.f };
			current_crosshair[1].pos = Vec2f{ 0.f, -2.f };
			current_crosshair[2].pos = Vec2f{ 2.f,  0.f };
			current_crosshair[3].pos = Vec2f{ 0.f,  2.f };

		}

		for (auto target : targets) {
			Vec2f pos = target->get_target_pos();
			if (!debug_draw::repeat((void*)target, pos)) {
				//LOG_INFO("{}", (void*)target);

				debug_draw::set_offset(pos);

				auto& target_crosshair = createDebugDrawable<VertexArray, debug_draw::Type::CAMERA_TARGET>(
					(const void*)target, Primitive::LINES, 4);

				for (int i = 0; i < target_crosshair.size(); i++) {
					target_crosshair[i].color = Color::White;
				}
				target_crosshair[0].pos = Vec2f{ -2.f,  0.f };
				target_crosshair[1].pos = Vec2f{  2.f,  0.f };
				target_crosshair[2].pos = Vec2f{  0.f, -2.f };
				target_crosshair[3].pos = Vec2f{  0.f,  2.f };

				debug_draw::set_offset();
			}
		}
	}


}


void GameCamera::addTarget(CameraTarget& target) {
	auto iter_pair = std::equal_range(
		targets.begin(), targets.end(), &target,
		[](const CameraTarget* lhs, const CameraTarget* rhs) {
			return lhs->get_priority() < rhs->get_priority();
		});

	bool first_target = targets.empty();

	targets.insert(iter_pair.second, &target);

	if (first_target) {
		targets.back()->m_state = CamTargetState::Active;
		active_target = targets.back();

		currentPosition = target.get_target_pos();
		prevPosition = currentPosition;
		deltaPosition = Vec2f{};
	}

	target.has_camera = true;
}
bool GameCamera::removeTarget(CameraTarget& target) {

	auto iter = std::find(targets.begin(), targets.end(), &target);
	bool has_target = iter != targets.end();
	if (has_target) {
		if (active_target == &target) {
			active_target = nullptr;
		}
		targets.erase(iter);
		target.has_camera = false;
	}
	return has_target;
}


void GameCamera::removeAllTargets() {
	for (auto target : targets) {
		target->has_camera = false;
	}
	targets.clear();
	active_target = nullptr;
}

const std::vector<CameraTarget*>& GameCamera::getTargets() const {
	return targets;
}


Vec2f GameCamera::getPosition(float interpolation) {
	return math::lerp(prevPosition, currentPosition, interpolation);
}

}