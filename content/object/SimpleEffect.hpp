#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/id_ptr.hpp"

#include <memory>

class SimpleEffect : public ff::GameObject {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	SimpleEffect(ff::World* world, ff::AnimID anim, ff::Vec2f position, bool hflip)
		: ff::GameObject(world)
		, scene_obj(world, world->create_scene_object({}))
	{
        scene_obj->drawable = ff::copyable_unique_ptr<ff::Drawable>{ new ff::AnimatedSprite() };
        sprite = static_cast<ff::AnimatedSprite*>(scene_obj->drawable.get());
		sprite->set_pos(position);
		sprite->set_pos(position);
		sprite->set_hflip(hflip);
		//m_remove = !sprite->set_anim(anim);
	};

	void update(secs deltaTime) override {
		sprite->update(deltaTime);
		if (sprite->is_complete()) {
			//m_remove = true;
		}
	};

	void predraw(float interp, bool updated) override {
		sprite->predraw(interp);
	};

	//ff::Scene_ptr<ff::AnimatedSprite> sprite;
    ff::unique_id<ff::SceneObject> scene_obj;
    ff::AnimatedSprite* sprite;
};
