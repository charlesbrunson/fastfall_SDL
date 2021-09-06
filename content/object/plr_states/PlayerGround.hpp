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
private:

	struct move_t {
		move_t(const Player& plr);

		float speed = 0.f;
		int movex = 0; 
		int wishx = 0;
	};

	void jump(Player& plr, const move_t& move);

	void accel(Player& plr, const move_t& move);

	void update_speed(Player& plr, const move_t& move);
	void update_anim(Player& plr, const move_t& move);
};
