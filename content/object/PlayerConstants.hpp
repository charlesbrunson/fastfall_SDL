#pragma once 

#include "fastfall/resource/asset/AnimAsset.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "fastfall/util/Default.hpp"

namespace plr::anim {

	extern ff::AnimIDRef idle;
	extern ff::AnimIDRef land;
	extern ff::AnimIDRef run;
	extern ff::AnimIDRef jump;
	extern ff::AnimIDRef fall;

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