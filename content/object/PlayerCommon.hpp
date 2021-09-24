#pragma once 


#include "Player.hpp"
#include "fastfall/util/Default.hpp"

namespace plr {
	struct move_t {
		move_t(const Player& plr);

		float speed = 0.f;
		int movex = 0;
		int wishx = 0;

		// relative to facing direction
		int facing = 0;
		float rel_speed = 0.f;
		int rel_movex = 0;
		int rel_wishx = 0;
	};
}

namespace plr::anim {

	extern ff::AnimIDRef idle;
	extern ff::AnimIDRef land;
	extern ff::AnimIDRef run;

	extern ff::AnimIDRef dash_n2;
	extern ff::AnimIDRef dash_n1;
	extern ff::AnimIDRef dash_0;
	extern ff::AnimIDRef dash_p1;
	extern ff::AnimIDRef dash_p2;

	extern ff::AnimIDRef jump;
	extern ff::AnimIDRef jump_f;
	extern ff::AnimIDRef jump_b;

	extern ff::AnimIDRef fall;
	extern ff::AnimIDRef fall_f;
	extern ff::AnimIDRef fall_b;

	extern ff::AnimIDRef brakeb;
	extern ff::AnimIDRef brakef;
}

namespace plr::constants {

	extern ff::Friction braking;
	extern ff::Friction moving;

	extern ff::Default<float> max_speed;
	extern ff::Default<float> norm_speed;
	extern ff::Default<float> jumpVelY;

	extern ff::Default<float> ground_accel;
	extern ff::Default<float> ground_high_decel;
	extern ff::Default<float> ground_idle_decel;

	extern ff::Default<ff::Vec2f> grav_normal;
	extern ff::Default<ff::Vec2f> grav_light;
}

namespace plr::action {
	PlayerStateID jump(Player& plr, const move_t& move);
	PlayerStateID dash(Player& plr, const move_t& move);
}
