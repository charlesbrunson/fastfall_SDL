#include "PlayerCommon.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

plr::move_t::move_t(const Player& plr)
{
	wishx = 0;
	if (Input::isHeld(InputType::RIGHT)) wishx++;
	if (Input::isHeld(InputType::LEFT))  wishx--;

	int flipper = (plr.sprite->get_hflip() ? -1 : 1);

	speed = plr.ground->traverse_get_speed();
	rel_speed = speed * flipper;

	movex = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));
	speed = abs(speed);

	rel_movex = movex * flipper;
	rel_wishx = wishx * flipper;

	facing = flipper;
}

namespace plr::anim {

	AnimIDRef idle("player", "idle");
	AnimIDRef land("player", "land");
	AnimIDRef run("player", "running");

	//AnimIDRef dash("player", "dash");
	AnimIDRef dash_n2("player", "dash-2");
	AnimIDRef dash_n1("player", "dash-1");
	AnimIDRef dash_0 ("player", "dash0");
	AnimIDRef dash_p1("player", "dash+1");
	AnimIDRef dash_p2("player", "dash+2");

	AnimIDRef jump("player", "jump");
	AnimIDRef jump_f("player", "jump_f");
	//AnimIDRef jump_b("player", "jump_b");

	AnimIDRef fall("player", "fall");
	AnimIDRef fall_f("player", "fall_f");
	//AnimIDRef fall_b("player", "fall_b");

	AnimIDRef brakeb("player", "brake_back");
	AnimIDRef brakef("player", "brake_front");
}

namespace plr::constants {

	Friction braking{ .stationary = 1.2f, .kinetic = 0.8f };
	Friction moving { .stationary = 0.0f, .kinetic = 0.0f };

	Default<float> max_speed = 500.f;
	Default<float> norm_speed = 180.f;
	Default<float> jumpVelY = -190.f;

	Default<float> ground_accel = 1200.f;
	Default<float> ground_high_decel = 300.f;
	Default<float> ground_idle_decel = 450.f;

	Default<Vec2f> grav_normal = Vec2f{ 0.f, 500.f };
	Default<Vec2f> grav_light = Vec2f{ 0.f, 350.f };
}

namespace plr::action {

	PlayerStateID dash(Player& plr, const move_t& move)
	{
		if (move.wishx != 0) {
			plr.sprite->set_hflip(move.wishx < 0);
		}
		return PlayerStateID::Dash;
	}

	PlayerStateID jump(Player& plr, const move_t& move)
	{
		Vec2f contact_normal = Vec2f{0.f, -1.f};
		Vec2f contact_velocity = Vec2f{};

		if (plr.ground->has_contact()) {
			contact_normal = plr.ground->get_contact()->collider_normal;
			contact_velocity = plr.ground->get_contact()->velocity;
		}


		float neutral_min_vx = -50.f;
		float neutral_max_vx = constants::norm_speed - 5.f;

		if (move.rel_speed > neutral_max_vx) 
		{
			// running jump

			plr.sprite->set_anim(anim::jump_f);
		}
		else if (move.rel_speed >= neutral_min_vx) {
			// neutral jump

			if (move.speed < 100.f && move.wishx != 0) {
				plr.ground->traverse_set_speed(plr.ground->traverse_get_speed() + 50.f * move.wishx);
			}
			plr.sprite->set_anim(anim::jump);

		}
		else if (move.rel_speed < neutral_min_vx)
		{
			//back jump
			if (move.rel_wishx < 0) {
				plr.sprite->set_hflip(!plr.sprite->get_hflip());
				plr.sprite->set_anim(anim::jump_f);
			}
			else {
				plr.sprite->set_anim(anim::jump);
			}
		}

		plr.ground->settings.slope_sticking = false;
		plr.ground->settings.slope_wall_stop = false;

		Vec2f jumpVel = Vec2f{ plr.box->get_vel().x, constants::jumpVelY };
		Angle jump_ang = math::angle(jumpVel) - math::angle(contact_normal);

		// from perpendicular to the ground
		static const Angle min_jump_ang = Angle::Degree(60);

		if (jump_ang < -min_jump_ang) {
			jumpVel = math::rotate(jumpVel, -jump_ang - min_jump_ang);
		}
		else if (jump_ang > min_jump_ang) {
			jumpVel = math::rotate(jumpVel, -jump_ang + min_jump_ang);
		}
		plr.box->set_vel(jumpVel + Vec2f{ 0.f, contact_velocity.y });

		return PlayerStateID::Air;
	}
}
