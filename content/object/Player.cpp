
#include "Player.hpp"
#include "PlayerConstants.hpp"

#include "plr_states/PlayerAir.hpp"
#include "plr_states/PlayerGround.hpp"

#include "fastfall/engine/input.hpp"

#include "fastfall/game/InstanceInterface.hpp"


#include <functional>

using namespace ff;
using namespace plr;

//using namespace plr_constants;

Player::Player(GameContext context, const ObjectRef& ref, const ObjectType& type) 
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
			.stick_angle_max = Angle::Degree(90),
		});

	// triggers
	hurtbox->set_trigger_callback(
		[](const TriggerPull& pull) {
			if (auto owner = pull.trigger.get().get_owner()) 
			{
				if (GameObject* obj = *owner; 
					obj->getType().group_tags.contains("player")
					&& pull.state == Trigger::State::Entry)
				{
					if (auto rpayload = obj->command<ObjCmd::GetPosition>().payload())
					{
						LOG_INFO("position: {}", rpayload->to_string());
					}

				}
			}
		});

	// sprite
	sprite->set_anim(plr::anim::idle);
	sprite->set_pos(box->getPosition());
};

std::unique_ptr<GameObject> Player::clone() const {

	std::unique_ptr<Player> object = std::make_unique<Player>(context, getObjectRef(), getType());

	//TODO copy current state data

	return object;
}

void Player::update(secs deltaTime) {




	if (curr_state) {
		PlayerStateID n_id = curr_state->update(*this, deltaTime);

		if (n_id != curr_state->get_id()) {

			std::unique_ptr<PlayerState> next_state;
			switch (n_id) {
			case PlayerStateID::Ground:	
				next_state = std::make_unique<PlayerGroundState>();
				break;
			case PlayerStateID::Air:	
				next_state = std::make_unique<PlayerAirState>();
				break;
			}

			curr_state->exit(*this, next_state.get());
			curr_state.swap(next_state);
			curr_state->enter(*this, next_state.get());
			//curr_state->update(deltaTime);
		}
	}

	/*
	if (Input::isPressed(InputType::ATTACK)) {
		box->setSize(Vec2f(box->getBox().getSize()) + Vec2f(1.f, 1.f));
	}
	if (Input::isPressed(InputType::DASH)) {
		box->setSize(Vec2f(box->getBox().getSize()) + Vec2f(-1.f, -1.f));
	}
	*/

	// blink
	if (Input::getMouseInView() && Input::isPressed(InputType::MOUSE1)) {
		box->teleport(Input::getMouseWorldPosition());
		box->set_vel(Vec2f{});
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
	ImGui::Text("Hello World!");
	ImGui::Text("Current State: %d %s", curr_state->get_id(), curr_state->get_name());
	ImGui::Text("Position(%3.2f, %3.2f)", box->getPosition().x, box->getPosition().y);
	ImGui::Text("Velocity(%3.2f, %3.2f)", box->get_vel().x, box->get_vel().y);
}

void Player::draw(RenderTarget& target, RenderState states) const {
	target.draw(*sprite, states);
}
