#include "PlayerConstants.hpp"

using namespace ff;

namespace plr::anim {

	AnimIDRef idle("player", "idle");
	AnimIDRef land("player", "land");
	AnimIDRef run("player", "running");
	AnimIDRef jump("player", "jump");
	AnimIDRef fall("player", "fall");

	AnimIDRef brakeb("player", "brake_back");
	AnimIDRef brakef("player", "brake_front");
}

namespace plr::constants {

	Friction braking{ .stationary = 1.2f, .kinetic = 0.8f };
	Friction moving{ .stationary = 0.f,  .kinetic = 0.f };

	Default<float> max_speed = 500.f;
	Default<float> norm_speed = 200.f;
	Default<float> jumpVelY = -200.f;

	Default<float> ground_accel = 1200.f;
	Default<float> ground_high_decel = 300.f;
	Default<float> ground_idle_decel = 450.f;

	Default<Vec2f> grav_normal = Vec2f{ 0.f, 500.f };
	Default<Vec2f> grav_light = Vec2f{ 0.f, 350.f };
}