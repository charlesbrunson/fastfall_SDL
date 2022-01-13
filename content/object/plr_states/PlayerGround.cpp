#include "PlayerGround.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

void PlayerGroundState::enter(plr::members& plr, PlayerState* from)
{
	plr.ground->settings.slope_sticking = true;
	plr.ground->settings.slope_wall_stop = true;

	float speed = plr.ground->traverse_get_speed();

	// set initial ground max speed
	plr.ground->settings.max_speed =
		math::clamp(std::abs(speed), constants::norm_speed.get(), constants::max_speed.get());

}

PlayerStateID PlayerGroundState::update(plr::members& plr, secs deltaTime)
{
	if (deltaTime <= 0.0) 
		return PlayerStateID::Continue;

	plr.sprite->set_playback(1.f);
	plr.box->set_gravity(constants::grav_normal);

	if (plr.ground->has_contact()) {

		plr.box->setSlipNone();
		const PersistantContact& contact = plr.ground->get_contact().value();
		plr.ground->settings.slope_sticking = true;

		move_t move{ plr };

		bool speeding = false;

		// clamp ground max speed to current speed and normal speed
		plr.ground->settings.max_speed =
			math::clamp(move.speed, constants::norm_speed.get(), plr.ground->settings.max_speed);

		// if we're going faster than normal, force decceleration
		if (move.speed > constants::norm_speed) {
			speeding = true;
			plr.ground->traverse_add_decel(constants::ground_high_decel);
		}

		auto accel = [&plr, &move](int dir) 
		{
			plr.ground->traverse_add_accel(dir * constants::ground_accel);
			plr.ground->settings.surface_friction = constants::moving;
		};

		auto brake = [&plr, &move](bool is_idle) 
		{
			if (move.speed > 100.f) {
				plr.sprite->set_anim(
					move.rel_movex < 0 ? anim::brakeb : anim::brakef
				);

				if (move.speed <= 300.f) {
					plr.sprite->set_frame(1);
				}
			}

			plr.ground->settings.surface_friction = constants::braking;

			plr.ground->traverse_add_decel(
				is_idle ? constants::ground_idle_decel : constants::ground_high_decel
			);
		};

		auto run = [&plr, &move]() 
		{
			plr.ground->traverse_add_accel(move.wishx * constants::ground_accel);
			plr.ground->settings.surface_friction = constants::moving;

			if (plr.sprite->get_hflip() != (move.wishx < 0)) {
				move.facing = !move.facing;
				move.rel_movex *= -1;
				move.rel_speed *= -1;
				move.rel_wishx *= -1;
				plr.sprite->set_hflip(move.wishx < 0);
			}

			if (!plr.sprite->is_playing(anim::run)) {
				if (move.speed < constants::norm_speed)
					plr.sprite->set_anim_if_not(anim::idle_to_run);
				else
					plr.sprite->set_anim_if_not(anim::run);
			}
			plr.sprite->set_playback(
				std::clamp(move.speed / 150.f, 0.5f, 1.5f)
			);
		};

		if (move.wishx != 0) 
		{
			if (move.movex != 0) 
			{
				if (move.rel_wishx == move.rel_movex) 
				{
					if (move.movex != move.facing && move.speed > 100.f) {
						brake(false);
					}
					else {
						run();
					}
				}
				else
				{
					if (move.speed < 25.f) {
						plr.ground->traverse_set_speed(move.wishx * 25.f);
					}
					brake(true);
				}
			}
			else {
				run();
			}
		}
		else 
		{
			if (move.movex != 0) 
			{
				if (move.speed < 25.f) 
				{
					plr.sprite->set_anim_if_not(anim::idle);
				}
				brake(false);
			}
			else 
			{
				plr.sprite->set_anim_if_not(anim::idle);
			}

			plr.ground->settings.surface_friction = constants::braking;
		}

		// jumping
		if (Input::isPressed(InputType::JUMP, 0.1f)) 
		{
			Input::confirmPress(InputType::JUMP);
			return action::jump(plr, move);
		}

		// dashing
		if (Input::isPressed(InputType::DASH, 0.25f))
		{
			Input::confirmPress(InputType::DASH);
			return action::dash(plr, move);
		}
		
	}
	else {
		return PlayerStateID::Air;
	}
	return PlayerStateID::Continue;
}

void PlayerGroundState::exit(plr::members& plr, PlayerState* to)
{
	plr.ground->settings.max_speed = 0.f;
}

void PlayerGroundState::get_imgui(plr::members& plr)
{
}
