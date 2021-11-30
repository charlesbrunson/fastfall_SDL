#pragma once

#include "../PlayerCommon.hpp"

class PlayerGroundState : public PlayerState {
public:
	void enter(plr::data_t& plr, PlayerState* from) override;
	PlayerStateID update(plr::data_t& plr, secs deltaTime) override;
	void exit(plr::data_t& plr, PlayerState* to) override;

	constexpr PlayerStateID get_id() const override {
		return PlayerStateID::Ground;
	}
	constexpr std::string_view get_name() const override {
		return "ground";
	}

	void get_imgui(plr::data_t& plr) override;
};
