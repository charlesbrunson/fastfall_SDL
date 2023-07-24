#include "PlayerAir.hpp"

#include "fastfall/engine/input/InputConfig.hpp"

using namespace ff;

using namespace plr;

void PlayerAirState::enter(ff::World& w, plr::members& plr, PlayerState* from)
{
    auto& box = w.at(plr.collidable_id);
	prevVelY = box.get_local_vel().y;
}

PlayerStateID PlayerAirState::update(ff::World& w, plr::members& plr, secs deltaTime)
{
	if (deltaTime <= 0.0)
		return PlayerStateID::Continue;

    auto [sprite, box] = w.at(plr.sprite_id, plr.collidable_id);
    auto& ground = *box.tracker();

	//int wishx = (int)Input::isHeld(InputType::RIGHT) - (int)Input::isHeld(InputType::LEFT);
    //int wishx = w.input().is_held(InputType::RIGHT) - w.input().is_held(InputType::LEFT);

	sprite.set_playback(1.f);
	box.set_gravity(constants::grav_normal);

	if (!ground.has_contact()) {

		ground.settings.slope_sticking = box.get_local_vel().y > -50.f;


		if (box.get_local_vel().y > -100.f)
		{
			box.setSlip({Collidable::SlipState::SlipVertical, 6.f});
		}

		if (abs(box.get_local_vel().y) < 50.f) {
			box.set_gravity(constants::grav_light);
		}
		else if (box.get_local_vel().y > 50.f)
		{
			float f  = (std::min)((box.get_local_vel().y - 50) / 100.f, 1.f);
			float nf = 1 - f;


			box.set_gravity(Vec2f{ 0.f, (f * 800.f) + (nf * Vec2f{constants::grav_light}.y) });
		}

		move_t move(w, plr);
		if (ground.air_time > 0.05) {

			if (!sprite.is_playing_any({ 
					anim::jump, anim::fall,
					anim::jump_f, anim::fall_f
				}))
			{
				sprite.set_anim(anim::fall);
				sprite.set_frame(2);
			}
			else if (box.get_local_vel().y > -100.f
				&& sprite.is_playing(anim::jump)
				) 
			{
				sprite.set_anim(anim::fall);
			}
			else if (box.get_local_vel().y > -100.f
				&& sprite.is_playing(anim::jump_f)
				) 
			{
				sprite.set_anim(anim::fall_f);
			}

			// idle jump turn around at apex
			if (sprite.is_playing(anim::fall)
				&& box.get_local_vel().y > -100.f
				&& prevVelY <= -100.f
				&& move.rel_movex < 0
				&& move.rel_wishx < 0)
			{
				sprite.set_hflip(!sprite.get_hflip());
			}
		}

		move = { w, plr };

		// air control
		if (move.rel_wishx != 0)
		{
			float air_control_hi = box.get_local_vel().y < 100.f && box.get_local_vel().y > -100.f ? 500.f : 300.f;
			float air_control_lo = box.get_local_vel().y < 100.f && box.get_local_vel().y > -100.f ? 400.f : 250.f;

			if (move.speed < 150.f)
			{
				box.add_accel({air_control_hi * move.wishx, 0.f});
			}
			else if (move.rel_speed >= 150.f == move.rel_wishx < 0) 
			{
				box.add_accel({air_control_lo * move.wishx, 0.f});
			}
		}
		else {
			box.add_decel({50.f, 0.f});
		}
		prevVelY = box.get_local_vel().y;
	}
	else {
		if (prevVelY > 150.f + ground.get_contact()->velocity.y) {
			sprite.set_anim(anim::land);
			sprite.set_frame(1);
		}
		else if (!sprite.is_playing_any(anim::get_ground_anims())) {
			sprite.set_anim(anim::idle);
		}
		return PlayerStateID::Ground;
	}
	return PlayerStateID::Continue;
}

PlayerStateID PlayerAirState::post_collision(ff::World& w, plr::members& plr)
{
    auto [sprite, box] = w.at(plr.sprite_id, plr.collidable_id);
    auto& ground = *box.tracker();
    
	if (ground.has_contact()) {
		if (prevVelY > 150.f + ground.get_contact()->velocity.y) {
			sprite.set_anim(anim::land);

			if (prevVelY < 350.f)
				sprite.set_frame(1);
		}
		else if (!sprite.is_playing_any(anim::get_ground_anims())) {
			sprite.set_anim(anim::land_soft);
		}
		ground.settings.slope_sticking = true;
		return PlayerStateID::Ground;
	}
	return PlayerStateID::Continue;
}

void PlayerAirState::exit(ff::World& w, plr::members& plr, PlayerState* to)
{
}
