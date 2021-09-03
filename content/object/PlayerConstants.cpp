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

	const Friction braking{ .stationary = 1.2f, .kinetic = 0.8f };
	const Friction moving{ .stationary = 0.f,  .kinetic = 0.f };

	const float max_speed = 500.f;
	const float norm_speed = 200.f;
	const float jumpVelY = -200.f;

	const Vec2f grav_normal{ 0.f, 500.f };
	const Vec2f grav_light{ 0.f, 350.f };
}