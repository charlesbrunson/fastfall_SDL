#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"

#include <memory>

class SimpleEffect : public ff::Object {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	SimpleEffect(ff::ActorInit init, const ff::AnimID& anim, ff::Vec2f position, bool hflip)
		: ff::Object(init)
        , anim_spr_id(init.world.create<ff::AnimatedSprite>(init.entity_id))
	{
        auto& spr = init.world.at(anim_spr_id);
		spr.set_pos(position);
		spr.set_hflip(hflip);
        dead = !spr.set_anim(anim);
	};

	void update(ff::World& w, secs deltaTime) override {
        auto& spr = w.at(anim_spr_id);
		spr.update(deltaTime);
        dead |= spr.is_complete();
	};

private:
    ff::ID<ff::AnimatedSprite> anim_spr_id;
};
