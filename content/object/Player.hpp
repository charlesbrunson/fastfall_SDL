#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "../camera/SimpleCamTarget.hpp"

#include <string_view>

//using namespace ff;

class PlayerState;

enum class PlayerStateID {
	Continue,
	Ground,
	Air,
	Dash
};

class Player : public ff::GameObject {
public:
	Player(ff::GameContext instance, const ff::ObjectData& ref, const ff::ObjectType& type);
	std::unique_ptr<ff::GameObject> clone() const override;

	void update(secs deltaTime) override;
	void predraw(secs deltaTime) override;

	void ImGui_Inspect() override;

	ff::Scene_ptr<ff::AnimatedSprite> sprite;
	ff::Collidable_ptr box;
	ff::SurfaceTracker* ground;
	ff::Trigger_ptr hurtbox;
	ff::Trigger_ptr hitbox;
	SimpleCamTarget cam_target;

protected:
	std::unique_ptr<PlayerState> curr_state;

	template<typename T>
	requires std::is_base_of_v<PlayerState, T>
	void state_transition();

	void manage_state(PlayerStateID n_id);

	CmdResponse do_command(ff::ObjCmd cmd, const std::any& payload) override;

};

class PlayerState {
public:
	virtual ~PlayerState() {};

	virtual void enter(Player& plr, PlayerState* from) {};
	virtual PlayerStateID update(Player& plr, secs deltaTime) = 0;
	virtual void exit(Player& plr, PlayerState* to) {};

	virtual constexpr PlayerStateID get_id() const = 0;
	virtual constexpr std::string_view get_name() const = 0;

	virtual PlayerStateID post_collision(Player& plr) { return PlayerStateID::Continue; };

	virtual void get_imgui(Player& plr) {};
};

template<typename T>
	requires std::is_base_of_v<PlayerState, T>
void Player::state_transition() {
	std::unique_ptr<PlayerState> next_state = 
		std::make_unique<T>();

	curr_state->exit(*this, next_state.get());
	curr_state.swap(next_state);
	curr_state->enter(*this, next_state.get());
}

