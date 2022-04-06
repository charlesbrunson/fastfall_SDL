#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"

#include <memory>

class SimpleEffect : public ff::GameObject {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	SimpleEffect(ff::GameContext context, ff::AnimID anim, ff::Vec2f position, bool hflip)
		: ff::GameObject(context)
		, sprite(context, ff::AnimatedSprite{}, ff::SceneType::Object, 0, ff::SceneManager::Priority::High)
	{
		sprite->set_pos(position);
		sprite->set_hflip(hflip);
		m_remove = !sprite->set_anim(anim);
	};

	std::unique_ptr<ff::GameObject> clone() const override
	{
		return nullptr;		
	};

	void update(secs deltaTime) override {
		sprite->update(deltaTime);
		if (sprite->is_complete()) {
			m_remove = true;
		}
	};

	void predraw(float interp) override {
		sprite->predraw(interp);
	};

	ff::Scene_ptr<ff::AnimatedSprite> sprite;
};
