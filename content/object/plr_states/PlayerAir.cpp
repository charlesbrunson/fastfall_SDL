#include "PlayerAir.hpp"
#include "../PlayerCommon.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

void PlayerAirState::enter(Player& plr, PlayerState* from) 
{
}

PlayerStateID PlayerAirState::update(Player& plr, secs deltaTime) 
{
	if (deltaTime <= 0.0)
		return PlayerStateID::Continue;

	int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

	plr.sprite->set_playback(1.f);
	plr.box->set_gravity(constants::grav_normal);

	if (!plr.ground->has_contact()) {

		plr.ground->settings.slope_sticking = plr.box->get_vel().y > -50.f;


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
					anim::jump_f, anim::fall_f
				}))
			{
				plr.sprite->set_anim(anim::fall);
				plr.sprite->set_frame(2);
			}
			else if (plr.box->get_vel().y > -100.f
				&& plr.sprite->is_playing(anim::jump)
				) 
			{
				plr.sprite->set_anim(anim::fall);
			}
			else if (plr.box->get_vel().y > -100.f
				&& plr.sprite->is_playing(anim::jump_f)
				) 
			{
				plr.sprite->set_anim(anim::fall_f);
			}
		}

		move_t move(plr);

		// air control
		if (move.rel_wishx != 0)
		{
			float air_control_hi = plr.box->get_vel().y < 100.f && plr.box->get_vel().y > -100.f ? 500.f : 300.f;
			float air_control_lo = plr.box->get_vel().y < 100.f && plr.box->get_vel().y > -100.f ? 400.f : 250.f;


			if (move.speed < 150.f )
			{
				plr.box->add_accelX(air_control_hi * move.wishx);
			}
			else if (move.rel_speed >= 150.f && move.rel_wishx < 0) 
			{
				plr.box->add_accelX(air_control_lo * move.wishx);
			}
		}
		else {
			plr.box->add_decelX(50.f);
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
