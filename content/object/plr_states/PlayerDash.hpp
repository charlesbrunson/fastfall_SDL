#pragma once

#include "../PlayerCommon.hpp"

class PlayerDashState : public PlayerState {
public:
	void enter(plr::members& plr, PlayerState* from) override;
	PlayerStateID update(plr::members& plr, secs deltaTime) override;
	void exit(plr::members& plr, PlayerState* to) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Dash;
	}
	constexpr std::string_view get_name() const override {
		return "dash";
	}
protected:
	secs dash_time = 0.0;
	secs dash_air_time = 0.0;

	float dash_speed = 0.f;

	bool ground_flag = false;
};
