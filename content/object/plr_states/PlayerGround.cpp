#include "PlayerGround.hpp"
#include "../PlayerConstants.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

PlayerGroundState::move_t::move_t(const Player& plr)
{
	wishx = 0;
	if (Input::isHeld(InputType::RIGHT)) wishx++;
	if (Input::isHeld(InputType::LEFT))  wishx--;

	speed = plr.ground->traverse_get_speed();
	movex = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));
	speed = abs(speed);
}

void PlayerGroundState::enter(Player& plr, PlayerState* from)
{
	plr.ground->settings.slope_sticking = true;

	float speed = plr.ground->traverse_get_speed();
	plr.ground->settings.max_speed =
		math::clamp(std::abs(speed), constants::norm_speed.get(), plr.ground->settings.max_speed);
}

PlayerStateID PlayerGroundState::update(Player& plr, secs deltaTime)
{

	plr.sprite->set_playback(1.f);
	plr.box->set_gravity(constants::grav_normal);

	if (plr.ground->has_contact()) {

		plr.box->setSlipNone();
		const PersistantContact& contact = plr.ground->get_contact().value();
		plr.ground->settings.slope_sticking = true;

		move_t move{ plr };

		update_speed(plr, move);

		update_anim(plr, move);

		// jumping
		if (Input::isPressed(InputType::JUMP, 0.1f)) 
		{
			Input::confirmPress(InputType::JUMP);
			jump(plr, move);
			return PlayerStateID::Air;
		}
		else 
		{
			accel(plr, move);
		}
	}
	else {
		return PlayerStateID::Air;
	}
	return PlayerStateID::Continue;
}

void PlayerGroundState::update_speed(Player& plr, const move_t& move)
{
	plr.ground->settings.max_speed = 
		math::clamp(move.speed, constants::norm_speed.get(), plr.ground->settings.max_speed);

	if (move.speed > constants::norm_speed) {
		plr.ground->traverse_add_decel(constants::ground_high_decel);
	}
}

void PlayerGroundState::update_anim(Player& plr, const move_t& move)
{
	if (move.speed <= 100.f && move.wishx == 0
		|| move.speed <= 5.f && move.wishx != 0) {
		plr.sprite->set_anim_if_not(anim::idle);
	}
	else {
		if ((move.wishx < 0) == (move.movex < 0)) {
			if (move.wishx != 0) {
				plr.sprite->set_hflip(move.movex < 0);
			}

			plr.sprite->set_anim_if_not(anim::run);
			plr.sprite->set_playback(
				std::clamp(move.speed / 150.f, 0.5f, 2.f)
			);
		}
	}
}

void PlayerGroundState::jump(Player& plr, const move_t& move)
{

	plr.sprite->set_anim(anim::jump);
	plr.ground->settings.slope_sticking = false;

	Vec2f jumpVel = Vec2f{ plr.box->get_vel().x, constants::jumpVelY };
	Angle jump_ang = math::angle(jumpVel) - math::angle(plr.ground->get_contact()->collider_normal);

	// from perpendicular to the ground
	static const Angle min_jump_ang = Angle::Degree(60);

	if (jump_ang < -min_jump_ang) {
		jumpVel = math::rotate(jumpVel, -jump_ang - min_jump_ang);
	}
	else if (jump_ang > min_jump_ang) {
		jumpVel = math::rotate(jumpVel, -jump_ang + min_jump_ang);
	}
	plr.box->set_vel(jumpVel + Vec2f{ 0.f, plr.ground->get_contact()->velocity.y });

	if (move.wishx != 0) {
		plr.sprite->set_hflip(move.wishx < 0);
	}
}

void PlayerGroundState::accel(Player& plr, const move_t& move)
{
	bool turning = move.wishx != 0 && ((move.movex < 0) != (move.wishx < 0));
	bool nowishx = move.wishx == 0;
	bool shouldbrake = nowishx || turning;

	if (move.movex != 0 && move.speed > 100.f && shouldbrake) {
		AnimID brakeStyle =
			plr.sprite->get_hflip() == (move.movex < 0) ?
			anim::brakef.id() :
			anim::brakeb.id();

		if (move.speed > 300.f) {
			plr.sprite->set_anim(brakeStyle);
		}
		else if (!(plr.sprite->is_playing(brakeStyle) && plr.sprite->get_frame() == 0)) {
			plr.sprite->set_anim(brakeStyle);
			plr.sprite->set_frame(1);
		}
	}

	if (move.wishx != 0) {
		plr.ground->traverse_add_accel(move.wishx * constants::ground_accel);
		plr.ground->settings.surface_friction =
			turning || nowishx ?
			constants::braking :
			constants::moving;
	}
	else if (plr.ground->settings.has_friction) {
		plr.ground->traverse_add_decel(constants::ground_idle_decel);
		plr.ground->settings.surface_friction = constants::braking;
	}
}

void PlayerGroundState::exit(Player& plr, PlayerState* to)
{
}
