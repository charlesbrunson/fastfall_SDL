#include "PlayerDash.hpp"

#include "fastfall/engine/input.hpp"

#include "../SimpleEffect.hpp"

using namespace ff;

using namespace plr;

constexpr secs dash_duration = 0.24;

//constexpr float dash_vel = 220.f;

float get_dash_vel(float min_speed) {

	//float ratio = dash_time / dash_duration;
	//float velx = (start_velx * (1 - ratio)) + (end_velx * (ratio));
	return std::max(min_speed, plr::constants::dash_speed.get());
}

void apply_dash_vel(ff::World& w, plr::members& plr, float min_vel) {

    auto& sprite = w.at_drawable<AnimatedSprite>(plr.sprite_scene_id);
    auto& box = w.at(plr.collidable_id);
    auto& ground = w.at_tracker(plr.collidable_id, plr.surfacetracker_id);

	float vel = min_vel;
	float speed = ground.has_contact()
		? *ground.traverse_get_speed()
		: box.get_vel().x;

	if (speed == 0 || speed < 0 == sprite.get_hflip())
	{
		vel = std::max(min_vel, abs(speed));
	}

	if (ground.has_contact()) {
		ground.traverse_set_speed(vel * (sprite.get_hflip() ? -1.f : 1.f));
	}
	else {
		box.set_vel(vel, {});
	}

}

PlayerStateID dash_jump(ff::World& w, plr::members& plr, const move_t& move) {
	return action::jump(w, plr, move);
}

struct dash_anims {
	const AnimIDRef* dash;
	const AnimIDRef* fx;
};

dash_anims select_dash_anim(ff::World& w, const plr::members& plr)
{
	dash_anims anims{ &anim::dash_0, &anim::fx::dash_0 };

    auto& sprite = w.at_drawable<AnimatedSprite>(plr.sprite_scene_id);
    auto& box = w.at(plr.collidable_id);
    auto& ground = w.at_tracker(plr.collidable_id, plr.surfacetracker_id);

	if (ground.has_contact()) {
		Vec2f cNorm = ground.get_contact()->collider_n;
		Angle ang = math::angle(cNorm) + Angle::Radian((float)PI_F / 2.f);

		if (sprite.get_hflip()) {
			ang = -ang;
		}

		//instance::obj_make<SimpleEffect>(plr.getContext(), anim::fx::dash_0.id(), Vec2f{ plr.box->getPosition() }, plr.sprite->get_hflip());
		if (ang.degrees() > 40.f) {
			anims.dash  = &anim::dash_n2;
			anims.fx = &anim::fx::dash_n2;
		}
		else if (ang.degrees() > 20.f) {
			anims.dash = &anim::dash_n1;
			anims.fx = &anim::fx::dash_n1;
		}
		else if (ang.degrees() == 0.f) {
			//anims.dash = &anim::dash_0;
			//anims.fx = &anim::fx::dash_0;
		}
		else if (ang.degrees() < -40.f) {
			anims.dash = &anim::dash_p2;
			anims.fx = &anim::fx::dash_p2;
		}
		else if (ang.degrees() < -20.f) {
			anims.dash = &anim::dash_p1;
			anims.fx = &anim::fx::dash_p1;
		}
	}
	return anims;
}

void PlayerDashState::enter(ff::World& w, plr::members& plr, PlayerState* from)
{

    auto& sprite = w.at_drawable<AnimatedSprite>(plr.sprite_scene_id);
    auto& box = w.at(plr.collidable_id);
    auto& ground = w.at_tracker(plr.collidable_id, plr.surfacetracker_id);

	ground_flag = ground.has_contact();
	
	if (ground_flag) {

		auto dash_anims = select_dash_anim(w, plr);
		if (sprite.set_anim_if_not(dash_anims.dash->id()))
		{
			Vec2f pos = box.getPosition();
			//ObjectFactory::create<SimpleEffect>(&w, dash_anims.fx->id(), pos, sprite.get_hflip());
            w.create_object<SimpleEffect>(dash_anims.fx->id(), pos, sprite.get_hflip());
		}
		dash_speed = *ground.traverse_get_speed() * (sprite.get_hflip() ? -1.f : 1.f);
	}
	ground.settings.slope_wall_stop = false;
	ground.settings.use_surf_vel = true;
}

PlayerStateID PlayerDashState::update(ff::World& w, plr::members& plr, secs deltaTime)
{
	if (deltaTime <= 0.0)
		return PlayerStateID::Continue;

    auto& sprite = w.at_drawable<AnimatedSprite>(plr.sprite_scene_id);
    auto& box = w.at(plr.collidable_id);
    auto& ground = w.at_tracker(plr.collidable_id, plr.surfacetracker_id);

	if (ground.has_contact()) {
		box.setSlip({});
		ground_flag = true;
		box.set_gravity(constants::grav_normal);
		apply_dash_vel(w, plr, get_dash_vel(dash_speed));

		auto dash_anims = select_dash_anim(w, plr);
		sprite.set_anim_if_not(dash_anims.dash->id());

		if (Input::isPressed(InputType::JUMP, 0.1f))
		{
			Input::confirmPress(InputType::JUMP);
			return dash_jump(w, plr, move_t(w, plr));
		}


		dash_time += deltaTime;
		if (dash_time >= dash_duration) {
			if (ground.has_contact()) {
				return PlayerStateID::Ground;
			}

			sprite.set_anim(anim::fall_f);
			sprite.set_frame(1);
			return PlayerStateID::Air;
		}
	}
	else {
		box.setSlip({Collidable::SlipState::SlipVertical, 6.f});
		box.set_gravity(Vec2f{});
		if (ground_flag) {

			if (Input::isPressed(InputType::JUMP, 0.1f))
			{
				Input::confirmPress(InputType::JUMP);
				return dash_jump(w, plr, move_t(w, plr));
			}

			dash_air_time += deltaTime;
			if (dash_air_time >= 0.1) {
				sprite.set_anim(anim::fall_f);
				sprite.set_frame(1);
				return PlayerStateID::Air;
			}
		}
		else if (!ground_flag) {
			apply_dash_vel(w, plr, get_dash_vel(dash_speed));
		}
	}

	return PlayerStateID::Continue;
}

void PlayerDashState::exit(ff::World& w, plr::members& plr, PlayerState* to)
{
    auto& box = w.at(plr.collidable_id);
    auto& ground = w.at_tracker(plr.collidable_id, plr.surfacetracker_id);

	box.set_gravity(constants::grav_normal);

	if (to->get_id() == PlayerStateID::Ground) {
		apply_dash_vel(w, plr, plr::constants::dash_speed);
	}

	ground.settings.use_surf_vel = true;
}
