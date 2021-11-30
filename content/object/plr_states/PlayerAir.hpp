#pragma once

#include "../PlayerCommon.hpp"

class PlayerAirState : public PlayerState {
public:
	void enter(plr::data_t& plr, PlayerState* from) override;
	PlayerStateID update(plr::data_t& plr, secs deltaTime) override;
	void exit(plr::data_t& plr, PlayerState* to) override;

	PlayerStateID post_collision(plr::data_t& plr) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Air;
	}
	constexpr std::string_view get_name() const override {
		return "air";
	}
protected:
	float prevVelY;
};
