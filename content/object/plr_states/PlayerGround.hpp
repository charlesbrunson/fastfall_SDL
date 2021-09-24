#pragma once

#include "../Player.hpp"

class PlayerGroundState : public PlayerState {
public:
	void enter(Player& plr, PlayerState* from) override;
	PlayerStateID update(Player& plr, secs deltaTime) override;
	void exit(Player& plr, PlayerState* to) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Ground;
	}
	constexpr std::string_view get_name() const override {
		return "ground";
	}

	virtual void get_imgui(Player& plr);
};
