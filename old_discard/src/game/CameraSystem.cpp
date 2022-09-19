#include "fastfall/game/CameraSystem.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/engine/config.hpp"

#include "fastfall/game/InstanceInterface.hpp"
#include "fastfall/game/CameraSystem.hpp"


namespace ff {


CameraTarget::CameraTarget(CamTargetPriority priority)
	: m_priority(priority)
{
}

CameraTarget::~CameraTarget() {
	/*
	if (has_camera) {
		instance::cam_erase_target(m_context, *this);
	}
	*/
}

CameraSystem::CameraSystem()
{
	set_component_callbacks();
}


CameraSystem::CameraSystem(Vec2f initPos) :
	currentPosition(initPos)
{
	set_component_callbacks();
};

CameraSystem::CameraSystem(const CameraSystem& other)
{
	zoomFactor = other.zoomFactor;
	lockPosition = other.lockPosition;

	deltaPosition = other.deltaPosition;
	prevPosition = other.prevPosition;
	currentPosition = other.currentPosition;

	targets = other.targets;
	active_target = other.active_target;
	ordered_targets = other.ordered_targets;

	set_component_callbacks();
}

CameraSystem& CameraSystem::operator=(const CameraSystem& other)
{
	zoomFactor = other.zoomFactor;
	lockPosition = other.lockPosition;

	deltaPosition = other.deltaPosition;
	prevPosition = other.prevPosition;
	currentPosition = other.currentPosition;

	targets = other.targets;
	active_target = other.active_target;
	ordered_targets = other.ordered_targets;

	set_component_callbacks();
	return *this;
}

void CameraSystem::set_component_callbacks()
{
	targets.on_create = [this](ID<CameraTarget> id) 
	{
		add_to_ordered(id);
	};

	targets.on_erase = [this](ID<CameraTarget> id) 
	{
		std::erase(ordered_targets, id);
	};
}

void CameraSystem::update(secs deltaTime) {

	if (deltaTime > 0.0) {

		prevPosition = currentPosition;
		deltaPosition = Vec2f{};

		for (auto& target : ordered_targets) {
			targets.at(target).update(deltaTime);
		}

		if (active_target && *active_target != ordered_targets.back()) {

			get_active_target()->m_state = CamTargetState::Inactive;
			targets.at(ordered_targets.back()).m_state = CamTargetState::Active;
			active_target = ordered_targets.back();
		}
		else if (!active_target && ordered_targets.size() > 0) {
			targets.at(ordered_targets.back()).m_state = CamTargetState::Active;
			active_target = ordered_targets.back();
		}
		else if (ordered_targets.empty()) {
			active_target = {};
		}

		if (active_target && !lockPosition) {
			auto pos = get_active_target()->get_target_pos();
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

		for (auto& target_id : ordered_targets) 
		{

			auto& camtarget = targets.at(target_id);
			Vec2f pos = camtarget.get_target_pos();

			if (!debug_draw::repeat((void*)&camtarget, pos)) {
				//LOG_INFO("{}", (void*)target);

				debug_draw::set_offset(pos);

				auto& target_crosshair = createDebugDrawable<VertexArray, debug_draw::Type::CAMERA_TARGET>(
					(const void*)&camtarget, Primitive::LINES, 4);

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

void CameraSystem::add_to_ordered(ID<CameraTarget> id)
{
	auto iter_pair = std::equal_range(
		ordered_targets.begin(), ordered_targets.end(), id,
		[this](ID<CameraTarget> lhs, ID<CameraTarget> rhs) {
			const CameraTarget& lhs_ptr = targets.at(lhs);
			const CameraTarget& rhs_ptr = targets.at(rhs);
			return lhs_ptr.get_priority() < rhs_ptr.get_priority();
		});

	bool first_target = ordered_targets.empty();

	ordered_targets.insert(iter_pair.second, id);

	if (first_target) {
		auto& target = targets.at(ordered_targets.back());
		target.m_state = CamTargetState::Active;
		active_target = ordered_targets.back();

		currentPosition = target.get_target_pos();
		prevPosition = currentPosition;
		deltaPosition = Vec2f{};
	}

	targets.at(id).has_camera = true;
}

Vec2f CameraSystem::getPosition(float interpolation) {
	return math::lerp(prevPosition, currentPosition, interpolation);
}

}