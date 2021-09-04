#include "PlayerAir.hpp"
#include "../PlayerConstants.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

void PlayerAirState::enter(Player& plr, PlayerState* from) {

	//plr.ground->settings.has_friction = false;
}

PlayerStateID PlayerAirState::update(Player& plr, secs deltaTime) {

	int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

	plr.sprite->set_playback(1.f);
	plr.box->set_gravity(constants::grav_normal);

	if (!plr.ground->has_contact()) {


		if (plr.box->get_vel().y > -100.f) {
			plr.box->setSlipV(6.f);
		}
		else {
			plr.box->setSlipNone();
		}

		if (abs(plr.box->get_vel().y) < 50.f) {
			plr.box->set_gravity(constants::grav_light);
		}

		/*
		// flight control
		int wishy = (int)Input::isHeld(InputType::DOWN) - (int)Input::isHeld(InputType::UP);
		plr.box->add_accelY(600.f * wishy);
		*/

		if (plr.ground->get_air_time() > 0.05) {

			if (!plr.sprite->is_playing_any({ anim::jump, anim::fall }))
			{
				plr.sprite->set_anim(anim::fall);
				plr.sprite->set_frame(2);
			}
			else if (plr.sprite->is_complete(anim::jump)
				&& plr.box->get_vel().y > -100.f)
			{
				plr.sprite->set_anim(anim::fall);
			}
		}

		plr.ground->settings.slope_sticking = false;

		// air control
		if (wishx != 0 && (abs(plr.box->get_vel().x) < 150.f ||
			plr.box->get_vel().x < 0.f != wishx < 0.f)) {
			plr.box->add_accelX(500.f * wishx);
		}
	}
	else {
		return PlayerStateID::Ground;
	}
	return get_id();
}

void PlayerAirState::exit(Player& plr, PlayerState* to) {

	float speed = plr.ground->traverse_get_speed();
	plr.ground->settings.max_speed =
		std::max(constants::norm_speed,
			std::min(std::abs(speed), constants::max_speed)
		);
}