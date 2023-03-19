#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/render/drawable/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"

#include <memory>

class SimpleEffect : public ff::Object {
public:
    static const ff::ObjectType Type;
	SimpleEffect(ff::ActorInit init, const ff::AnimID& anim, ff::Vec2f position, bool hflip);
	void update(ff::World& w, secs deltaTime) override;

private:
    ff::ID<ff::AnimatedSprite> anim_spr_id;
};
