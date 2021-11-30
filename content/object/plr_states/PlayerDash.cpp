#include "PlayerDash.hpp"

#include "fastfall/engine/input.hpp"

#include "../SimpleEffect.hpp"

using namespace ff;

using namespace plr;

constexpr secs dash_duration = 0.24;

constexpr float start_velx = 240.f;
constexpr float end_velx = 240.f;

float get_dash_vel(secs dash_time, float min_speed) {

	float ratio = dash_time / dash_duration;
	float velx = (start_velx * (1 - ratio)) + (end_velx * (ratio));
	return std::max(min_speed, velx);
}

void apply_dash_vel(plr::data_t& plr, float min_vel) {

	float vel = min_vel;
	if (
		plr.ground->traverse_get_speed() == 0 ||
		plr.ground->traverse_get_speed() < 0 == plr.sprite->get_hflip()) 
	{
		vel = std::max(min_vel, abs(plr.ground->traverse_get_speed()));
	}

	plr.ground->traverse_set_speed(vel * (plr.sprite->get_hflip() ? -1.f : 1.f));

}

PlayerStateID dash_jump(plr::data_t& plr, const move_t& move) {
	return action::jump(plr, move);
}

struct dash_anims {
	const AnimIDRef* dash;
	const AnimIDRef* fx;
};

dash_anims select_dash_anim(const plr::data_t& plr)
{
	dash_anims anims{ &anim::dash_0, &anim::fx::dash_0 };

	if (plr.ground->has_contact()) {
		Vec2f cNorm = plr.ground->get_contact()->collider_normal;
		Angle ang = math::angle(cNorm) + Angle::Radian((float)PI_F / 2.f);

		if (plr.sprite->get_hflip()) {
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

void PlayerDashState::enter(plr::data_t& plr, PlayerState* from)
{

	ground_flag = plr.ground->has_contact();
	
	if (ground_flag) {

		auto dash_anims = select_dash_anim(plr);
		if (plr.sprite->set_anim_if_not(dash_anims.dash->id())) 
		{
			Vec2f pos = plr.box->getPosition();
			ObjectFactory::create<SimpleEffect>(plr.context, dash_anims.fx->id(), pos, plr.sprite->get_hflip());
		}
		dash_speed = plr.ground->traverse_get_speed();
	}

	//plr.ground->settings.slope_sticking = false;

	plr.ground->settings.slope_wall_stop = false;
	plr.ground->settings.use_surf_vel = true;
}

PlayerStateID PlayerDashState::update(plr::data_t& plr, secs deltaTime)
{
	if (deltaTime <= 0.0)
		return PlayerStateID::Continue;

	if (plr.ground->has_contact()) {
		plr.box->setSlipNone();
		ground_flag = true;
		plr.box->set_gravity(constants::grav_normal);
		apply_dash_vel(plr, get_dash_vel(dash_time, dash_speed));

		auto dash_anims = select_dash_anim(plr);
		plr.sprite->set_anim_if_not(dash_anims.dash->id());

		if (Input::isPressed(InputType::JUMP, 0.1f))
		{
			Input::confirmPress(InputType::JUMP);
			return dash_jump(plr, move_t(plr));
		}


		dash_time += deltaTime;
		if (dash_time >= dash_duration) {
			if (plr.ground->has_contact()) {
				return PlayerStateID::Ground;
			}

			plr.sprite->set_anim(anim::fall_f);
			plr.sprite->set_frame(1);
			return PlayerStateID::Air;
		}
	}
	else {
		plr.box->setSlipV(6.f);
		plr.box->set_gravity(Vec2f{});
		if (ground_flag) {

			if (Input::isPressed(InputType::JUMP, 0.1f))
			{
				Input::confirmPress(InputType::JUMP);
				return dash_jump(plr, move_t(plr));
			}

			dash_air_time += deltaTime;
			if (dash_air_time >= 0.1) {
				plr.sprite->set_anim(anim::fall_f);
				plr.sprite->set_frame(1);
				return PlayerStateID::Air;
			}
		}
		else if (!ground_flag) {
			apply_dash_vel(plr, get_dash_vel(dash_time, dash_speed));
		}
	}

	return PlayerStateID::Continue;
}

void PlayerDashState::exit(plr::data_t& plr, PlayerState* to)
{
	plr.box->set_gravity(constants::grav_normal);

	if (to->get_id() == PlayerStateID::Ground) {
		apply_dash_vel(plr, end_velx);
	}

	plr.ground->settings.use_surf_vel = true;
}