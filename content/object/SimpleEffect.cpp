#include "SimpleEffect.hpp"

using namespace ff;

const ObjectType SimpleEffect::Type {
    .name = { "SimpleEffect" }
};

SimpleEffect::SimpleEffect(ActorInit init, const AnimID& anim, Vec2f position, bool hflip)
    : Object(init, Type)
{
    auto spr = init.world.create<ff::AnimatedSprite>(init.entity_id);
    anim_spr_id = spr;
    spr->set_pos(position);
    spr->set_hflip(hflip);
    dead = !spr->set_anim(anim);
};

void SimpleEffect::update(World& w, secs deltaTime) {
    auto& spr = w.at(anim_spr_id);
    spr.update(deltaTime);
    dead |= spr.is_complete();
};
