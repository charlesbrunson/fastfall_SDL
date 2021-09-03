#pragma once 

#include "fastfall/resource/asset/AnimAsset.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"


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

	extern const ff::Friction braking;
	extern const ff::Friction moving;

	extern const float max_speed;
	extern const float norm_speed;
	extern const float jumpVelY;

	extern const ff::Vec2f grav_normal;
	extern const ff::Vec2f grav_light;
}