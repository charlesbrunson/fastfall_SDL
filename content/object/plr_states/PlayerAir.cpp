#include "PlayerAir.hpp"
#include "../PlayerConstants.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

void PlayerAirState::enter(Player& plr, PlayerState* from) 
{
}

PlayerStateID PlayerAirState::update(Player& plr, secs deltaTime) {

	int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

	plr.sprite->set_playback(1.f);
	plr.box->set_gravity(constants::grav_normal);

	if (!plr.ground->has_contact()) {
		plr.ground->settings.slope_sticking = false;


		if (plr.box->get_vel().y > -100.f) {
			plr.box->setSlipV(6.f);
		}
		else {
			plr.box->setSlipNone();
		}

		if (abs(plr.box->get_vel().y) < 50.f) {
			plr.box->set_gravity(constants::grav_light);
		}

		if (plr.ground->get_air_time() > 0.05) {

			if (!plr.sprite->is_playing_any({ 
					anim::jump, anim::fall,
					anim::jump_f, anim::fall_f,
				}))
			{
				plr.sprite->set_anim(anim::fall);
				plr.sprite->set_frame(2);
			}
			else if (plr.box->get_vel().y > -100.f
				&& plr.sprite->is_complete(anim::jump)) 
			{
				plr.sprite->set_anim(anim::fall);
			}
			else if (plr.box->get_vel().y > -150.f
				&& plr.sprite->is_complete(anim::jump_f)) 
			{
				plr.sprite->set_anim(anim::fall_f);
			}
		}


		// air control
		if (wishx != 0 && (abs(plr.box->get_vel().x) < 150.f ||
			plr.box->get_vel().x < 0.f != wishx < 0.f)) {
			plr.box->add_accelX(500.f * wishx);
		}
		prevVelY = plr.box->get_vel().y;
	}
	else {
		if (prevVelY > 150.f) {
			plr.sprite->set_anim(anim::land);
		}
		return PlayerStateID::Ground;
	}
	return PlayerStateID::Continue;
}

void PlayerAirState::exit(Player& plr, PlayerState* to) 
{
}