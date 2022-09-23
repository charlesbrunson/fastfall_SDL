#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"

#include <memory>

class SimpleEffect : public ff::GameObject {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	SimpleEffect(ff::World& world, const ff::AnimID& anim, ff::Vec2f position, bool hflip)
		: ff::GameObject(world)
		, scene_id(world.create_scene_object({}))
	{
        auto& scene_obj = world.at(scene_id);
        scene_obj.drawable = ff::copyable_unique_ptr<ff::Drawable>{ new ff::AnimatedSprite() };
        auto& spr = world.at_drawable<ff::AnimatedSprite>(scene_id);
		spr.set_pos(position);
		spr.set_pos(position);
		spr.set_hflip(hflip);
        raise_should_delete(!spr.set_anim(anim));
	};

	void update(ff::World& w, secs deltaTime) override {
        auto& spr = w.at_drawable<ff::AnimatedSprite>(scene_id);
		spr.update(deltaTime);
        raise_should_delete(spr.is_complete());
	};

	void predraw(ff::World& w, float interp, bool updated) override {
		w.at_drawable<ff::AnimatedSprite>(scene_id).predraw(interp);
	};

    void clean(ff::World& w) override {
        w.erase(scene_id);
    }

private:
    ff::ID<ff::SceneObject> scene_id;
};
