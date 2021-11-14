#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"

#include <memory>

class SimpleEffect : public ff::GameObject {
public:
	SimpleEffect(ff::ObjectConfig cfg, ff::AnimID anim, ff::Vec2f position, bool hflip)
		: ff::GameObject(cfg)
		, sprite(cfg.context, ff::AnimatedSprite{}, ff::SceneType::Object, 0, ff::SceneManager::Priority::High)
	{
		sprite->set_pos(position);
		sprite->set_hflip(hflip);
		toDelete = !sprite->set_anim(anim);
	};

	std::unique_ptr<GameObject> clone() const override 
	{
		return nullptr;		
	};

	void update(secs deltaTime) override {
		sprite->update(deltaTime);
		if (sprite->is_complete()) {
			toDelete = true;
		}
	};

	void predraw(secs deltaTime) override {
		sprite->predraw(deltaTime);
	};

	ff::Scene_ptr<ff::AnimatedSprite> sprite;
};