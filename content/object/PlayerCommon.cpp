#include "PlayerCommon.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

plr::move_t::move_t(const plr::members& plr)
{
	wishx = 0;
	if (Input::isHeld(InputType::RIGHT)) wishx++;
	if (Input::isHeld(InputType::LEFT))  wishx--;

	int flipper = (plr.sprite->get_hflip() ? -1 : 1);

	auto gspeed = plr.ground->traverse_get_speed();
	speed = gspeed ? *gspeed : plr.box->get_vel().x;
	rel_speed = speed * flipper;

	movex = (speed == 0.f ? 0 : (speed < 0.f ? -1 : 1));
	speed = abs(speed);

	rel_movex = movex * flipper;
	rel_wishx = wishx * flipper;

	facing = flipper;
}


plr::members::members(GameContext context_, GameObject& plr, Vec2f position)
	: box(context_, position, ff::Vec2f(8.f, 28.f), constants::grav_normal)
	, hitbox(context_, box->getBox(), { "hitbox" }, {}, &plr)
	, hurtbox(context_, box->getBox(), { "hurtbox" }, { "hitbox" }, &plr)
	, sprite(context_, AnimatedSprite{}, SceneType::Object)
	, cam_target(context_, CamTargetPriority::Medium, [&]() { return box->getPosition() + Vec2f{ 0.f, -16.f }; })
	, plr_context(context_)
{
}

namespace plr::anim {

	AnimIDRef idle("player.sax", "idle");
	AnimIDRef land("player.sax", "land");
	AnimIDRef idle_to_run("player.sax", "idle_to_run");
	AnimIDRef run("player.sax", "running");

	AnimIDRef dash_n2("player.sax", "dash-2");
	AnimIDRef dash_n1("player.sax", "dash-1");
	AnimIDRef dash_0 ("player.sax", "dash0");
	AnimIDRef dash_p1("player.sax", "dash+1");
	AnimIDRef dash_p2("player.sax", "dash+2");

	AnimIDRef jump("player.sax", "jump");
	AnimIDRef jump_f("player.sax", "jump_f");

	AnimIDRef fall("player.sax", "fall");
	AnimIDRef fall_f("player.sax", "fall_f");

	AnimIDRef brakeb("player.sax", "brake_back");
	AnimIDRef brakef("player.sax", "brake_front");


	namespace fx {
		AnimIDRef dash_n2("player.sax", "dash_fx-2");
		AnimIDRef dash_n1("player.sax", "dash_fx-1");
		AnimIDRef dash_0 ("player.sax", "dash_fx0");
		AnimIDRef dash_p1("player.sax", "dash_fx+1");
		AnimIDRef dash_p2("player.sax", "dash_fx+2");
	}

	const std::vector<AnimID>& get_ground_anims() {
		static std::vector<AnimID> ground_anims{
			idle, land, run, brakeb, brakef,
			dash_n2, dash_n1, dash_0, dash_p1, dash_p2,
		};
		return ground_anims;
	}

	const std::vector<AnimID>& get_air_anims() {
		static std::vector<AnimID> air_anims{
			jump, jump_f,
			fall, fall_f
		};
		return air_anims;
	}
}

namespace plr::constants {

	Friction braking{ .stationary = 1.2f, .kinetic = 0.6f };
	Friction moving { .stationary = 0.0f, .kinetic = 0.0f };

	Default<float> max_speed = 400.f;
	Default<float> norm_speed = 150.f;
	Default<float> jumpVelY = -190.f;

	Default<float> ground_accel = 1000.f;
	Default<float> ground_high_decel = 200.f;
	Default<float> ground_idle_decel = 300.f;

	Default<Vec2f> grav_normal = Vec2f{ 0.f, 500.f };
	Default<Vec2f> grav_light = Vec2f{ 0.f, 400.f };


	Default<float> dash_speed = 220.f;
}

namespace plr::action {

	PlayerStateID dash(plr::members& plr, const move_t& move)
	{
		if (move.wishx != 0) {
			plr.sprite->set_hflip(move.wishx < 0);
		}
		return PlayerStateID::Dash;
	}

	PlayerStateID jump(plr::members& plr, const move_t& move)
	{
		Vec2f contact_normal = Vec2f{0.f, -1.f};
		Vec2f contact_velocity = Vec2f{};

		if (plr.ground->has_contact()) {
			contact_normal = plr.ground->get_contact()->collider_n;
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
				if (plr.ground->has_contact()) {
					plr.ground->traverse_set_speed(*plr.ground->traverse_get_speed() + 50.f * move.wishx);
				}
				else {
					plr.box->set_vel(plr.box->get_vel().x + 50.f * move.wishx, {});
				}
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
