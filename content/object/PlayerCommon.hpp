#pragma once 

//#include "Player.hpp"
#include "fastfall/util/Default.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "../camera/SimpleCamTarget.hpp"

namespace plr::anim {

	extern ff::AnimIDRef idle;
	extern ff::AnimIDRef land;
	extern ff::AnimIDRef idle_to_run;
	extern ff::AnimIDRef run;

	extern ff::AnimIDRef dash_n2;
	extern ff::AnimIDRef dash_n1;
	extern ff::AnimIDRef dash_0;
	extern ff::AnimIDRef dash_p1;
	extern ff::AnimIDRef dash_p2;

	extern ff::AnimIDRef jump;
	extern ff::AnimIDRef jump_f;

	extern ff::AnimIDRef fall;
	extern ff::AnimIDRef fall_f;

	extern ff::AnimIDRef brakeb;
	extern ff::AnimIDRef brakef;

	const std::vector<ff::AnimID>& get_ground_anims();
	const std::vector<ff::AnimID>& get_air_anims();

	namespace fx {
		extern ff::AnimIDRef dash_n2;
		extern ff::AnimIDRef dash_n1;
		extern ff::AnimIDRef dash_0;
		extern ff::AnimIDRef dash_p1;
		extern ff::AnimIDRef dash_p2;
	}
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


namespace plr {
	struct members {

		members(ff::GameContext context, ff::GameObject& plr, ff::Vec2f position);

		const ff::GameContext plr_context;

		ff::Scene_ptr<ff::AnimatedSprite> sprite;
		ff::Collidable_ptr box;
		ff::SurfaceTracker* ground;
		ff::Trigger_ptr hurtbox;
		ff::Trigger_ptr hitbox;
		SimpleCamTarget cam_target;
	};

	struct move_t {
		move_t(const plr::members& plr);

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

enum class PlayerStateID {
	Continue,
	Ground,
	Air,
	Dash
};

namespace plr::action {
	PlayerStateID jump(plr::members& plr, const move_t& move);
	PlayerStateID dash(plr::members& plr, const move_t& move);
}


class PlayerState {
public:
	virtual ~PlayerState() {};

	virtual void enter(plr::members& plr, PlayerState* from) {};
	virtual PlayerStateID update(plr::members& plr, secs deltaTime) = 0;
	virtual void exit(plr::members& plr, PlayerState* to) {};

	virtual constexpr PlayerStateID get_id() const = 0;
	virtual constexpr std::string_view get_name() const = 0;

	virtual PlayerStateID post_collision(plr::members& plr) { return PlayerStateID::Continue; };

	virtual void get_imgui(plr::members& plr) {};
};

