#pragma once


#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/level/TileLayer.hpp"

#include <memory>

class TilePlatform : public ff::GameObject {
public:
    static const ff::ObjectType Type;
    const ff::ObjectType& type() const override { return Type; };

    TilePlatform(ff::World& world, ff::ID<ff::GameObject> id);

    void update(ff::World& w, secs deltaTime) override;

private:

};

