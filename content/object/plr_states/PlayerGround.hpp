#pragma once

#include "../PlayerCommon.hpp"

class PlayerGroundState : public PlayerState {
public:
	void enter(plr::members& plr, PlayerState* from) override;
	PlayerStateID update(plr::members& plr, secs deltaTime) override;
	void exit(plr::members& plr, PlayerState* to) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Ground;
	}
	constexpr std::string_view get_name() const override {
		return "ground";
	}

	void get_imgui(plr::members& plr) override;
};
