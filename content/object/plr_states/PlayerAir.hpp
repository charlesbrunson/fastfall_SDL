#pragma once

#include "../PlayerCommon.hpp"

class PlayerAirState : public PlayerState {
public:
	void enter(plr::members& plr, PlayerState* from) override;
	PlayerStateID update(plr::members& plr, secs deltaTime) override;
	void exit(plr::members& plr, PlayerState* to) override;

	PlayerStateID post_collision(plr::members& plr) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Air;
	}
	constexpr std::string_view get_name() const override {
		return "air";
	}
protected:
	float prevVelY;
};
