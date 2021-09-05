#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "../camera/SimpleCamTarget.hpp"

#include <string_view>

//using namespace ff;

class PlayerState;

class Player : public ff::GameObject {
public:
	Player(ff::GameContext instance, const ff::ObjectRef& ref, const ff::ObjectType& type);
	std::unique_ptr<ff::GameObject> clone() const override;

	void update(secs deltaTime) override;
	void predraw(secs deltaTime) override;

	void ImGui_Inspect() override;

	enum class State {
		Ground,
		Air
	};

	ff::Scene_ptr<ff::AnimatedSprite> sprite;
	ff::Collidable_ptr box;
	ff::SurfaceTracker* ground;
	ff::Trigger_ptr hurtbox;
	ff::Trigger_ptr hitbox;
	SimpleCamTarget cam_target;

protected:
	std::unique_ptr<PlayerState> curr_state;

	CmdResponse do_command(ff::ObjCmd cmd, const std::any& payload) override;

	void draw(ff::RenderTarget& target, ff::RenderState states = ff::RenderState()) const override;
};

enum class PlayerStateID {
	Ground,
	Air
};

class PlayerState {
public:
	virtual ~PlayerState() {};

	virtual void enter(Player& plr, PlayerState* from) {};
	virtual PlayerStateID update(Player& plr, secs deltaTime) = 0;
	virtual void exit(Player& plr, PlayerState* to) {};

	virtual constexpr PlayerStateID get_id() const = 0;
	virtual constexpr std::string_view get_name() const = 0;
};
