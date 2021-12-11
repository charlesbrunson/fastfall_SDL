#include "PlayerAir.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

void PlayerAirState::enter(plr::members& plr, PlayerState* from)
{
}

PlayerStateID PlayerAirState::update(plr::members& plr, secs deltaTime)
{
	if (deltaTime <= 0.0)
		return PlayerStateID::Continue;

	int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);

	plr.sprite->set_playback(1.f);
	plr.box->set_gravity(constants::grav_normal);

	if (!plr.ground->has_contact()) {

		plr.ground->settings.slope_sticking = plr.box->get_vel().y > -50.f;


		if (plr.box->get_vel().y > -100.f) 
		{
			plr.box->setSlipV(6.f);
		}

		if (abs(plr.box->get_vel().y) < 50.f) {
			plr.box->set_gravity(constants::grav_light);
		}
		else if (plr.box->get_vel().y > 50.f)
		{
			float f  = std::min((plr.box->get_vel().y - 50) / 100.f, 1.f);
			float nf = 1 - f;


			plr.box->set_gravity(Vec2f{ 0.f, (f * 800.f) + (nf * Vec2f{constants::grav_light}.y) });
		}

		move_t move(plr);
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

			// idle jump turn around at apex
			if (plr.sprite->is_playing(anim::fall)
				&& plr.box->get_vel().y > -100.f
				&& plr.box->get_prev_vel().y <= -100.f
				&& move.rel_movex < 0
				&& move.rel_wishx < 0)
			{
				plr.sprite->set_hflip(!plr.sprite->get_hflip());
			}
		}



		move = move_t(plr);

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
		if (prevVelY > 150.f + plr.ground->get_contact()->velocity.y) {
			plr.sprite->set_anim(anim::land);
		}
		else if (!plr.sprite->is_playing_any(anim::get_ground_anims())) {
			plr.sprite->set_anim(anim::idle);
		}
		return PlayerStateID::Ground;
	}
	return PlayerStateID::Continue;
}

PlayerStateID PlayerAirState::post_collision(plr::members& plr)
{
	if (plr.ground->has_contact()) {
		if (prevVelY > 150.f + plr.ground->get_contact()->velocity.y) {
			plr.sprite->set_anim(anim::land);
		}
		else if (!plr.sprite->is_playing_any(anim::get_ground_anims())) {
			plr.sprite->set_anim(anim::idle);
		}
		plr.ground->settings.slope_sticking = true;
		return PlayerStateID::Ground;
	}
	return PlayerStateID::Continue;
}

void PlayerAirState::exit(plr::members& plr, PlayerState* to)
{
}
