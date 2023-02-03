#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"

#include <memory>

class SimpleEffect : public ff::Object {
public:
	SimpleEffect(ff::ActorInit init, const ff::AnimID& anim, ff::Vec2f position, bool hflip);
	void update(ff::World& w, secs deltaTime) override;

private:
    ff::ID<ff::AnimatedSprite> anim_spr_id;
};
