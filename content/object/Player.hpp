#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/game/phys/Collidable.hpp"

#include "fastfall/engine/input.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/game/phys/Raycast.hpp"

#include "fastfall/game/CollisionManager.hpp"
#include "fastfall/game/GameCamera.hpp"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/resource/Resources.hpp"

using namespace ff;

class Player : public GameObject {
public:
	Player(GameContext instance, const ObjectRef& ref);

	~Player();

	std::unique_ptr<GameObject> clone() const override;

	void update(secs deltaTime) override;

	void predraw(secs deltaTime) override;

	void ImGui_Inspect() override;

protected:

	secs airtime = 0.0;

	AnimatedSprite sprite;

	Friction braking;
	Friction moving;

	SurfaceTracker ground;

	float airNeutralVel;

	Collidable* box;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;
};
