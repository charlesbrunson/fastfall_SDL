#pragma once 

//#include "Player.hpp"
#include "fastfall/util/Default.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"
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

	extern ff::Default<float> dash_speed;
}


namespace plr {
	struct members {

		members(ff::World& w, ff::GameObject& plr, ff::Vec2f position);

        ff::ID<ff::SceneObject> sprite_scene_id;
        ff::ID<ff::Collidable> collidable_id;
        ff::ID<ff::SurfaceTracker> surfacetracker_id;
        ff::ID<ff::Trigger> hurtbox_id;
        ff::ID<ff::Trigger> hitbox_id;
        ff::ID<SimpleCamTarget> cameratarget_id;

	};

	struct move_t {
		move_t(ff::World& w, const plr::members& plr);

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
	PlayerStateID jump(ff::World& w, plr::members& plr, const move_t& move);
	PlayerStateID dash(ff::World& w, plr::members& plr, const move_t& move);
}


class PlayerState {
public:
	virtual ~PlayerState() {};

	virtual void enter(ff::World& w, plr::members& plr, PlayerState* from) {};
	virtual PlayerStateID update(ff::World& w, plr::members& plr, secs deltaTime) = 0;
	virtual void exit(ff::World& w, plr::members& plr, PlayerState* to) {};

	virtual constexpr PlayerStateID get_id() const = 0;
	virtual constexpr std::string_view get_name() const = 0;

	virtual PlayerStateID post_collision(ff::World& w, plr::members& plr) { return PlayerStateID::Continue; };

	virtual void get_imgui(plr::members& plr) {};
};

