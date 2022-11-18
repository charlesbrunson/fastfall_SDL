#include "PlayerGround.hpp"

#include "fastfall/engine/InputConfig.hpp"

using namespace ff;

using namespace plr;

void PlayerGroundState::enter(ff::World& w, plr::members& plr, PlayerState* from)
{
    auto& box = w.at(plr.collidable_id);
    auto& ground = box.at_tracker();
	ground.settings.slope_sticking = true;
	ground.settings.slope_wall_stop = true;

	auto gspeed = ground.traverse_get_speed();
	float speed = gspeed ? *gspeed : box.get_vel().x;
	//float speed = plr.ground->traverse_get_speed();

	// set initial ground max speed
	ground.settings.max_speed =
		math::clamp(std::abs(speed), constants::norm_speed.get(), constants::max_speed.get());

}

PlayerStateID PlayerGroundState::update(ff::World& w, plr::members& plr, secs deltaTime)
{
	if (deltaTime <= 0.0) 
		return PlayerStateID::Continue;

    auto& sprite = w.at(plr.sprite_id);
    auto& box = w.at(plr.collidable_id);
    auto& ground = box.at_tracker();

	sprite.set_playback(1.f);
	box.set_gravity(constants::grav_normal);

	if (ground.has_contact()) {

		box.setSlip({});
		const AppliedContact& contact = ground.get_contact().value();
		ground.settings.slope_sticking = true;

		move_t move{ w, plr };

		bool speeding = false;

		// clamp ground max speed to current speed and normal speed
		ground.settings.max_speed =
			math::clamp(move.speed, constants::norm_speed.get(), ground.settings.max_speed);

		// if we're going faster than normal, force decceleration
		if (move.speed > constants::norm_speed && ground.settings.has_friction) {
			speeding = true;
			ground.traverse_add_decel(constants::ground_high_decel);
		}

		auto accel = [&](int dir) 
		{
			ground.traverse_add_accel(dir * constants::ground_accel);
			ground.settings.surface_friction = constants::moving;
		};

		auto brake = [&](bool is_idle)
		{

			bool heavy_brake = false;
			if (move.speed > 100.f) {

				sprite.set_anim(
					move.rel_movex < 0 ? anim::brakeb : anim::brakef
				);

				if (move.speed <= 250.f) {
					sprite.set_frame(1);
				}
				heavy_brake = sprite.get_frame() == 0;
			}

			ground.settings.surface_friction = constants::braking;

			if (ground.settings.has_friction) {
				ground.traverse_add_decel(
					(is_idle ? constants::ground_idle_decel : constants::ground_high_decel)
					 * (heavy_brake ? 0.5f : 1.f)
				);
			}
		};

		auto run = [&]() 
		{
			ground.traverse_add_accel(move.wishx * constants::ground_accel);
			ground.settings.surface_friction = constants::moving;

			if (sprite.get_hflip() != (move.wishx < 0)) {
				move.facing	   *= -1;
				move.rel_movex *= -1;
				move.rel_speed *= -1;
				move.rel_wishx *= -1;
				sprite.set_hflip(move.wishx < 0);
			}

			if (!sprite.is_playing(anim::run)) {
				if (move.speed < constants::norm_speed)
					sprite.set_anim_if_not(anim::idle_to_run);
				else
					sprite.set_anim_if_not(anim::run);
			}
			sprite.set_playback(
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
						ground.traverse_set_speed(move.wishx * 25.f);
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
					sprite.set_anim_if_not(anim::idle);
				}
				brake(false);
			}
			else 
			{
				sprite.set_anim_if_not(anim::idle);
			}

			ground.settings.surface_friction = constants::braking;
		}

		// jumping
		if (w.input()[InputType::JUMP].is_pressed(0.1))
		{
			w.input()[InputType::JUMP].confirm_press();
			return action::jump(w, plr, move);
		}

		// dashing
		if (w.input()[InputType::DASH].is_pressed(0.25))
		{
            w.input()[InputType::DASH].confirm_press();
			return action::dash(w, plr, move);
		}
		
	}
	else {
		return PlayerStateID::Air;
	}
	return PlayerStateID::Continue;
}

void PlayerGroundState::exit(ff::World& w, plr::members& plr, PlayerState* to)
{
    auto& ground = w.at(plr.collidable_id).at_tracker();
	ground.settings.max_speed = 0.f;
}

void PlayerGroundState::get_imgui(plr::members& plr)
{
}
