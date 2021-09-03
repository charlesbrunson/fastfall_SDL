#pragma once

#include "../Player.hpp"

class PlayerAirState : public PlayerState {
public:
	void enter(Player& plr, PlayerState* from) override;
	PlayerStateID update(Player& plr, secs deltaTime) override;
	void exit(Player& plr, PlayerState* to) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Air;
	}
	constexpr std::string_view get_name() const override {
		return "air";
	}
};
