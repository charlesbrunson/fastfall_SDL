#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"

#include <memory>

class SimpleEffect : public ff::GameObject {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	SimpleEffect(ff::World& world, ff::ID<ff::GameObject> id, const ff::AnimID& anim, ff::Vec2f position, bool hflip)
		: ff::GameObject(world, id)
        , anim_spr_id(world.create_drawable<ff::AnimatedSprite>(id))
	{
        auto& spr = world.at(anim_spr_id);
		spr.set_pos(position);
		spr.set_hflip(hflip);
        raise_should_delete(!spr.set_anim(anim));
	};

	void update(ff::World& w, secs deltaTime) override {
        auto& spr = w.at(anim_spr_id);
		spr.update(deltaTime);
        raise_should_delete(spr.is_complete());
	};

private:
    ff::ID<ff::AnimatedSprite> anim_spr_id;
};
