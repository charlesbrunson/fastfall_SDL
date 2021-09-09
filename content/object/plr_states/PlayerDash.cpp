#include "PlayerDash.hpp"
#include "../PlayerCommon.hpp"

#include "fastfall/engine/input.hpp"

using namespace ff;

using namespace plr;

constexpr secs dash_duration = 0.20;

constexpr float start_velx = 240.f;
constexpr float end_velx = 240.f;

float get_dash_vel(secs dash_time) {

	float ratio = dash_time / dash_duration;
	return (start_velx * (1 - ratio)) + (end_velx * (ratio));
}

void apply_dash_vel(Player& plr, float min_vel) {

	float vel = min_vel;
	if (
		plr.ground->traverse_get_speed() == 0 ||
		plr.ground->traverse_get_speed() < 0 == plr.sprite->get_hflip()) {
		vel = std::max(min_vel, abs(plr.ground->traverse_get_speed()));
	}

	plr.ground->traverse_set_speed(vel * (plr.sprite->get_hflip() ? -1.f : 1.f));

}

void PlayerDashState::enter(Player& plr, PlayerState* from)
{

	//LOG_INFO("DASH START");
	ground_flag = plr.ground->has_contact();
	
	if (ground_flag) {
		plr.sprite->set_anim(anim::dash);
	}

	plr.ground->settings.slope_sticking = false;
	plr.ground->settings.use_surf_vel = false;
}

PlayerStateID PlayerDashState::update(Player& plr, secs deltaTime)
{
	if (plr.ground->has_contact()) {
		ground_flag = true;
		plr.box->set_gravity(constants::grav_normal);
		apply_dash_vel(plr, get_dash_vel(dash_time));

		if (Input::isPressed(InputType::JUMP, 0.1f))
		{
			Input::confirmPress(InputType::JUMP);
			return action::jump(plr, move_t(plr));
		}
	}
	else {
		plr.box->set_gravity(Vec2f{});
		if (ground_flag && dash_time > 0.1) {
			return PlayerStateID::Air;
		}
		else if (!ground_flag) {
			apply_dash_vel(plr, get_dash_vel(dash_time));
		}
	}


	dash_time += deltaTime;
	if (dash_time < dash_duration) {
		return PlayerStateID::Continue;
	}
	else {
		if (Input::isPressed(InputType::DASH, 0.25f))
		{
			Input::confirmPress(InputType::DASH);
			return action::dash(plr, move_t(plr));
		}
		return ground_flag ? PlayerStateID::Ground : PlayerStateID::Air;
	}
}

void PlayerDashState::exit(Player& plr, PlayerState* to)
{
	if (to->get_id() == PlayerStateID::Ground) {
		apply_dash_vel(plr, end_velx);
	}

	plr.ground->settings.slope_sticking = true;
	plr.ground->settings.use_surf_vel = true;
}