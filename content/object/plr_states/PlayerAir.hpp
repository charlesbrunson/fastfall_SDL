#pragma once

#include "../PlayerCommon.hpp"

class PlayerAirState : public PlayerState {
public:
	void enter(ff::World& w, plr::members& plr, PlayerState* from) override;
	PlayerStateID update(ff::World& w, plr::members& plr, secs deltaTime) override;
	void exit(ff::World& w, plr::members& plr, PlayerState* to) override;

	PlayerStateID post_collision(ff::World& w, plr::members& plr) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Air;
	}
	constexpr std::string_view get_name() const override {
		return "air";
	}
protected:
	float prevVelY;
};
