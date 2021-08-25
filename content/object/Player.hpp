#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/game/phys/Collidable.hpp"

#include "fastfall/engine/input.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/game/phys/Raycast.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/resource/Resources.hpp"

#include "fastfall/game/trigger/Trigger.hpp"

#include <cmath>

using namespace ff;

class Player : public GameObject {
public:
	Player(GameContext instance, const ObjectRef& ref, const ObjectType& type);

	~Player();

	std::unique_ptr<GameObject> clone() const override;

	void update(secs deltaTime) override;

	void predraw(secs deltaTime) override;

	void ImGui_Inspect() override;



protected:

	CmdResponse do_command(ObjCmd cmd, const std::any& payload) override {
		if (constexpr auto CMD = ObjCmd::NoOp; cmd == CMD)
		{
			LOG_INFO("It's me!");
			return respond<CMD>(true);
		}
		return GameObject::do_command(cmd, payload);
	}

	AnimatedSprite sprite;

	Collidable* box;
	SurfaceTracker* ground;

	Trigger* hurtbox;
	Trigger* hitbox;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;
};
