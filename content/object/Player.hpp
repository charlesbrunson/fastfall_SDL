#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "../camera/SimpleCamTarget.hpp"

#include <string_view>
#include <variant>

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
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	Player(ff::GameContext context, ff::Vec2f position, bool faceleft);

	Player(ff::GameContext context, ff::ObjectLevelData& data);

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
	struct plr_state_impl;
	std::unique_ptr<plr_state_impl> state_pImpl;

	void manage_state(PlayerStateID n_id);

	CmdResponse do_command(ff::ObjCmd cmd, const std::any& payload) override;

	void init();
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
