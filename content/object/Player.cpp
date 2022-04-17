#include "Player.hpp"
#include "PlayerCommon.hpp"

#include "fastfall/engine/input.hpp"

#include "fastfall/game/InstanceInterface.hpp"


#include <functional>

using namespace ff;
using namespace plr;

const ObjectType Player::Type{
	.type = { "Player" },
	.allow_as_level_data = true,
	.anim = std::make_optional(AnimIDRef{"player.sax", "idle"}),
	.tile_size = { 1u, 2u },
	.group_tags = {	"player" },
	.properties = {
		{ "faceleft",	 false },
		{ "anotherprop", ObjectPropertyType::String }
	}
};

Player::Player(GameContext context, Vec2f position, bool faceleft)
	: GameObject{ context }
	, plr::members{ context, *this, position }
{
	init();
	sprite->set_hflip(faceleft);
};

Player::Player(GameContext context, ObjectLevelData& data)
	: GameObject( context, data )
	, plr::members{ context, *this, Vec2f{ data.position } }
{
	init();
	sprite->set_hflip(data.getPropAsBool("faceleft"));
};


void Player::init() {

	// surface tracker
	ground = &box->create_tracker(
		Angle::Degree(-135), 
		Angle::Degree(-45),
		SurfaceTracker::Settings{
			.move_with_platforms = true,
			.slope_sticking = true,
			.slope_wall_stop = true,
			.has_friction = true,
			.use_surf_vel = true,
			.stick_angle_max = Angle::Degree(90),
			.max_speed = constants::norm_speed,
			.slope_stick_speed_factor = 0.f,
		});

	instance::cam_add_target(context(), cam_target);

	// trigger testing
	/*
	hurtbox->set_trigger_callback(
		[this](const TriggerPull& pull) {
			if (auto owner = pull.trigger->get_owner();
				owner 
				&& owner->type().group_tags.contains("player")
				&& pull.state == Trigger::State::Entry)
			{
				if (auto rpayload = owner->command<ObjCmd::GetPosition>().payload())
				{
					LOG_INFO("position: {}", rpayload->to_string());

					if (owner->command<ObjCmd::Hurt>(100.f).is_accepted()) {
						LOG_INFO("gottem");
					}
				}
			}
		});
	*/

	sprite->set_anim(plr::anim::idle);
	sprite->set_pos(box->getPosition());

	box->set_onPostCollision(
		[this] {
			manage_state(get_state().post_collision(*this));
		});
}

std::unique_ptr<GameObject> Player::clone() const 
{
	std::unique_ptr<Player> object = std::make_unique<Player>(m_context, box->getPosition(), sprite->get_hflip());

	// TODO

	return object;
}


void Player::manage_state(PlayerStateID n_id) 
{
	auto state_transition = [this]<std::derived_from<PlayerState> T>( T&& next_state ) 
	{
		get_state().exit(*this, &next_state);
		auto& prev_state = get_state();
		state = next_state;
		get_state().enter(*this, &prev_state);
	};

	switch (n_id) {
	case PlayerStateID::Continue:
		break;
	case PlayerStateID::Ground:
		state_transition(PlayerGroundState{});
		break;
	case PlayerStateID::Air:
		state_transition(PlayerAirState{});
		break;
	case PlayerStateID::Dash:
		state_transition(PlayerDashState{});
		break;
	}
}

void Player::update(secs deltaTime) {
	manage_state(get_state().update(*this, deltaTime));

	box->update(deltaTime);
	sprite->update(deltaTime);
	hitbox->update(box->getBox());
	hurtbox->update();
}

Player::CmdResponse Player::do_command(ObjCmd cmd, const std::any& payload) 
{
	return Behavior{ cmd, payload }
		.match<ObjCmd::NoOp>(
			[]() { return true; })
		.match<ObjCmd::GetPosition>(
			[this]() { return box->getPosition(); })
		.match<ObjCmd::Hurt>(
			[this](float damage) { LOG_INFO("OUCH: {}", damage); })
		;
}

void Player::predraw(float interp, bool updated) {

	sprite->set_pos(math::lerp(box->getPrevPosition(), box->getPosition(), interp));
	sprite->predraw(interp);
}

void Player::ImGui_Inspect() {
	using namespace ImGui;

	std::pair<PlayerStateID, std::string_view> pair = visit_state(
		[](PlayerState& state) { 
			return make_pair(state.get_id(), state.get_name());
		});
	auto id = pair.first;
	auto name = pair.second;

	ImGui::Text("Current State: %d %s", (int)id, name.data());
	ImGui::Text("Position(%3.2f, %3.2f)", box->getPosition().x, box->getPosition().y);
	ImGui::Text("Velocity(%3.2f, %3.2f)", box->get_vel().x, box->get_vel().y);

	Separator();

	DragFloat("Max Speed", &constants::max_speed.get(), 1.f, 0.f, 1000.f);
	SameLine(GetWindowWidth() - 60);
	if (SmallButton("reset##1")) { 
		constants::max_speed.restore();
	}

	if (DragFloat("Normal Speed", &constants::norm_speed.get(), 1.f, 0.f, 500.f)) {
		if (constants::norm_speed > ground->settings.max_speed) {
			ground->settings.max_speed = constants::norm_speed;
		}
	}
	SameLine(GetWindowWidth() - 60);
	if (SmallButton("reset##2")) {
		constants::norm_speed.restore();
	}

	DragFloat("Jump Vel", &constants::jumpVelY.get(), 1.f, -1000.f, 0.f);
	SameLine(GetWindowWidth() - 60);
	if (SmallButton("reset##3")) { 
		constants::jumpVelY.restore();
	}

	Separator();

	get_state().get_imgui(*this);
}
