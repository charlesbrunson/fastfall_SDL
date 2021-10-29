#include "Player.hpp"
#include "PlayerCommon.hpp"

#include "plr_states/PlayerAir.hpp"
#include "plr_states/PlayerGround.hpp"
#include "plr_states/PlayerDash.hpp"

#include "fastfall/engine/input.hpp"

#include "fastfall/game/InstanceInterface.hpp"


#include <functional>

using namespace ff;
using namespace plr;

//using namespace plr_constants;

Player::Player(GameContext context, const ObjectData& ref, const ObjectType& type)
	: GameObject{context, ref, type }
	, box(		 context, Vec2f(ref.position), Vec2f(8.f, 28.f), constants::grav_normal)
	, hitbox(	 context, box->getBox(), { "hitbox" }, {}, this)
	, hurtbox(	 context, box->getBox(), { "hurtbox" }, { "hitbox" }, this)
	, sprite(	 context, AnimatedSprite{}, SceneType::Object)
	, cam_target(context, CamTargetPriority::Medium, &box->getPosition(), Vec2f{ 0.f, -16.f })
	, curr_state(new PlayerGroundState())
{

	// surface tracker
	ground = &box->create_tracker(
		Angle::Degree(-135), Angle::Degree(-45), 
		{
			.move_with_platforms = true,
			.slope_sticking = true,
			.slope_wall_stop = true,
			.has_friction = true,
			.use_surf_vel = true,
			.stick_angle_max = Angle::Degree(90),
			.max_speed = constants::norm_speed,
		});

	// triggers
	hurtbox->set_trigger_callback(
		[](const TriggerPull& pull) {
			if (auto owner = pull.trigger->get_owner();
				pull.state == Trigger::State::Entry
				&& owner.has_value() 
				&& owner.value()->getType().group_tags.contains("player"))
			{
				GameObject* obj = *owner;
				if (auto rpayload = obj->command<ObjCmd::GetPosition>().payload())
				{
					LOG_INFO("position: {}", rpayload->to_string());
				}
			}
		});

	sprite->set_anim(plr::anim::idle);
	sprite->set_pos(box->getPosition());

	box->set_onPostCollision([this] {
			if (curr_state) {
				manage_state(curr_state->post_collision(*this));
			}
		});
};

std::unique_ptr<GameObject> Player::clone() const {

	std::unique_ptr<Player> object = std::make_unique<Player>(context, getObjectRef(), getType());

	//TODO copy current state data

	return object;
}


void Player::manage_state(PlayerStateID n_id) {
	switch (n_id) {
	case PlayerStateID::Continue:
		break;
	case PlayerStateID::Ground:
		state_transition<PlayerGroundState>();
		break;
	case PlayerStateID::Air:
		state_transition<PlayerAirState>();
		break;
	case PlayerStateID::Dash:
		state_transition<PlayerDashState>();
		break;
	}
}

void Player::update(secs deltaTime) {

	if (curr_state) {
		manage_state(curr_state->update(*this, deltaTime));
	}

	box->update(deltaTime);
	sprite->update(deltaTime);
	hitbox->update(box->getBox());
	hurtbox->update();
}

Player::CmdResponse Player::do_command(ObjCmd cmd, const std::any& payload) 
{
	if (const auto CMD = ObjCmd::NoOp; cmd == CMD)
	{
		return respond<CMD>(true);
	}
	if (const auto CMD = ObjCmd::GetPosition; cmd == CMD)
	{
		return respond<CMD>(box->getPosition());
	}
	return GameObject::do_command(cmd, payload);
}

void Player::predraw(secs deltaTime) {
	sprite->set_pos(box->getPosition());
	sprite->predraw(deltaTime);
}

void Player::ImGui_Inspect() {
	using namespace ImGui;

	Text("Current State: %d %s", curr_state->get_id(), curr_state->get_name().data());
	Text("Position(%3.2f, %3.2f)", box->getPosition().x, box->getPosition().y);
	Text("Velocity(%3.2f, %3.2f)", box->get_vel().x, box->get_vel().y);

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

	if (curr_state) {
		curr_state->get_imgui(*this);
	}
}

void Player::draw(RenderTarget& target, RenderState states) const {
	target.draw(*sprite, states);
}
