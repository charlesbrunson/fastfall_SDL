#pragma once

#include "fastfall/game/World.hpp"
// #include "fastfall/util/direction.hpp"

class Crawler : public ff::Actor {
public:
    const static ff::ActorType actor_type;

    Crawler(ff::ActorInit init, ff::Vec2f position, ff::Cardinal surface_dir, bool face_left);
    Crawler(ff::ActorInit init, const ff::LevelObjectData& data);

    void update(ff::World& w, secs deltaTime) override;

private:
    int8_t move_dir = 1;
    ff::Cardinal surface_dir;
    ff::ID<ff::AnimatedSprite> spr_id;
    ff::ID<ff::Collidable>     col_id;
};
