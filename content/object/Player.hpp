#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/game/phys/Collidable.hpp"

#include "fastfall/engine/input.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/AnimatedSprite.hpp"

#include "fastfall/game/object/ObjectComponents.hpp"

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

	CmdResponse do_command(ObjCmd cmd, const std::any& payload) override;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

	Scene_ptr<AnimatedSprite> sprite;

	Collidable_ptr box;
	SurfaceTracker* ground;

	Trigger_ptr hurtbox;
	Trigger_ptr hitbox;

};
