#include "PlayerGround.hpp"
#include "../PlayerCommon.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

void PlayerGroundState::enter(Player& plr, PlayerState* from)
{
	plr.ground->settings.slope_sticking = true;

	float speed = plr.ground->traverse_get_speed();

	// set initial ground max speed
	plr.ground->settings.max_speed =
		math::clamp(std::abs(speed), constants::norm_speed.get(), constants::max_speed.get());

}

struct ground_move_t 
{
	ground_move_t(Player& plr) 
	{
		if (plr.ground->traverse_get_speed() != 0.f) {
			r_move = plr.ground->traverse_get_speed() > 0 ? 1 : -1;

			if (plr.sprite->get_hflip()) 
				r_move *= -1;
		}

		r_wish = 0;
		if (Input::isHeld(InputType::RIGHT)) r_wish++;
		if (Input::isHeld(InputType::LEFT))  r_wish--;

		if (plr.sprite->get_hflip()) 
			r_wish *= -1;
	}

	int r_move = 0;
	int r_wish = 0;

	bool is_braking_f() { return r_move > 0 && r_wish <= 0; }
	bool is_braking_b() { return r_move < 0 && r_wish >= 0; }

};


PlayerStateID PlayerGroundState::update(Player& plr, secs deltaTime)
{
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

		auto accel = [&plr](int dir) {
			plr.ground->traverse_add_accel(dir * constants::ground_accel);
			plr.ground->settings.surface_friction = constants::moving;
		}; 

		auto brake = [&plr, &move](bool hard) 
		{
			if (move.speed > 100.f) {
				plr.sprite->set_anim(
					move.rel_movex < move.rel_wishx ? anim::brakeb : anim::brakef
				);
			}
			plr.ground->settings.surface_friction = constants::braking;

			plr.ground->traverse_add_decel(
				hard ? constants::ground_high_decel : constants::ground_idle_decel
			);
		};

		if (move.wishx != 0) {

			if (move.movex != 0) {

				if (move.rel_wishx == move.rel_movex) 
				{
					plr.ground->settings.surface_friction = constants::moving;

					plr.sprite->set_hflip(move.wishx < 0);
					accel(move.wishx);
					plr.sprite->set_anim_if_not(anim::run);
					plr.sprite->set_playback(
						std::clamp(move.speed / 150.f, 0.5f, 2.f)
					);
				}
				else  
				{
					brake(true);
				}
			}
			else {
				plr.sprite->set_hflip(move.wishx < 0);
				accel(move.wishx);
				plr.sprite->set_anim_if_not(anim::run);
			}
		}
		else {

			if (move.movex != 0) {

				if (move.rel_wishx == move.rel_movex) 
				{
					plr.sprite->set_anim_if_not(anim::idle);
				}
				else 
				{
					brake(false);
				}
			}
			else {
				plr.sprite->set_anim_if_not(anim::idle);
			}

			plr.ground->settings.surface_friction = constants::braking;
		}



		//update_anim(plr, move);
		/*
		if (move.speed <= 100.f && move.wishx == 0
			|| move.speed <= 5.f && move.wishx != 0) 
		{
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
		*/


		/*
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
		*/



		// jumping
		if (Input::isPressed(InputType::JUMP, 0.1f)) 
		{
			Input::confirmPress(InputType::JUMP);
			return action::jump(plr, move);
		}

		// dashing
		/*
		if (Input::isPressed(InputType::DASH, 0.25f))
		{
			Input::confirmPress(InputType::DASH);
			return action::dash(plr, move);
		}
		*/
	}
	else {
		return PlayerStateID::Air;
	}
	return PlayerStateID::Continue;
}

void PlayerGroundState::exit(Player& plr, PlayerState* to)
{
	plr.ground->settings.max_speed = 0.f;
}

void PlayerGroundState::get_imgui(Player& plr)
{
	ground_move_t m(plr);
	ImGui::Text("r_move: %d", m.r_move);
	ImGui::Text("r_wish: %d", m.r_wish);
	ImGui::Text("braking_f: %d", m.is_braking_f());
	ImGui::Text("braking_b: %d", m.is_braking_b());
}

void accel(Player& plr, const move_t& move)
{

	int r_move = 0;
	int r_wish = 0;

	if (plr.ground->traverse_get_speed() != 0.f) {
		r_move = plr.ground->traverse_get_speed() > 0 ? 1 : -1;

		if (plr.sprite->get_hflip())
			r_move *= -1;
	}

	r_wish = 0;
	if (Input::isHeld(InputType::RIGHT)) r_wish++;
	if (Input::isHeld(InputType::LEFT))  r_wish--;

	if (plr.sprite->get_hflip())
		r_wish *= -1;

	bool is_braking_f = r_move > 0 && r_wish <= 0;
	bool is_braking_b = r_move < 0 && r_wish >= 0;



	/*
	*/
}

