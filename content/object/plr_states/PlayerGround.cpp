#include "PlayerGround.hpp"
#include "../PlayerConstants.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

void PlayerGroundState::enter(Player& plr, PlayerState* from)
{
	//plr.ground->settings.has_friction = true;
	plr.ground->settings.slope_sticking = true;
}

PlayerStateID PlayerGroundState::update(Player& plr, secs deltaTime)
{
	int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

	plr.sprite->set_playback(1.f);
	plr.box->set_gravity(constants::grav_normal);

	if (plr.ground->has_contact()) {
		plr.box->setSlipNone();
		const PersistantContact& contact = plr.ground->get_contact().value();

		plr.ground->settings.slope_sticking = true;

		float speed = plr.ground->traverse_get_speed();

		plr.ground->settings.max_speed =
			std::max(constants::norm_speed,
				std::min(std::abs(speed), plr.ground->settings.max_speed)
			);

		if (abs(speed) > constants::norm_speed) {
			plr.ground->traverse_add_decel(300.f);
		}

		int movex = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));

		if (abs(speed) <= 100.f && wishx == 0
			|| abs(speed) <= 5.f && wishx != 0) {
			plr.sprite->set_anim_if_not(anim::idle);
		}
		else {


			if ((wishx < 0) == (movex < 0)) {
				if (wishx != 0) {
					plr.sprite->set_hflip(movex < 0);
				}

				plr.sprite->set_anim_if_not(anim::run);
				plr.sprite->set_playback(
					std::clamp(abs(speed) / 150.f, 0.5f, 2.f)
				);
			}
		}

		// jumping
		if (Input::isPressed(InputType::JUMP, 0.1f)) {
			Input::confirmPress(InputType::JUMP);

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


			if (wishx != 0) {
				plr.sprite->set_hflip(wishx < 0);
			}
			return PlayerStateID::Air;
		}
		else {

			bool turning = wishx != 0 && ((movex < 0) != (wishx < 0));
			bool nowishx = wishx == 0;
			bool shouldbrake = nowishx || turning;

			if (movex != 0 && abs(speed) > 100.f && shouldbrake) {
				AnimID brakeStyle =
					plr.sprite->get_hflip() == (movex < 0) ?
					anim::brakef.id() :
					anim::brakeb.id();

				if (abs(speed) > 300.f) {
					plr.sprite->set_anim(brakeStyle);
				}
				else if (!(plr.sprite->is_playing(brakeStyle) && plr.sprite->get_frame() == 0)) {
					plr.sprite->set_anim(brakeStyle);
					plr.sprite->set_frame(1);
				}
			}

			if (wishx != 0) {
				plr.ground->traverse_add_accel(wishx * 1200.f);
				plr.ground->settings.surface_friction =
					turning || nowishx ?
					constants::braking :
					constants::moving;
			}
			else if (plr.ground->settings.has_friction) {
				plr.ground->traverse_add_decel(450.f);
				plr.ground->settings.surface_friction = constants::braking;
			}
		}
	}
	else {
		return PlayerStateID::Air;
	}
	return get_id();
}

void PlayerGroundState::exit(Player& plr, PlayerState* to)
{
}
