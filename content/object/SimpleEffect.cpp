#include "SimpleEffect.hpp"

SimpleEffect::SimpleEffect(ff::ObjectInit init, const ff::AnimID& anim, ff::Vec2f position, bool hflip)
    : ff::Object(init)
{
    auto spr = init.world.create<ff::AnimatedSprite>(init.entity_id);
    anim_spr_id = spr;
    spr->set_pos(position);
    spr->set_hflip(hflip);
    dead = !spr->set_anim(anim);
};

void SimpleEffect::update(ff::World& w, secs deltaTime) {
    auto& spr = w.at(anim_spr_id);
    spr.update(deltaTime);
    dead |= spr.is_complete();
};