#pragma once


#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/level/TileLayer.hpp"

#include <memory>

class TilePlatform : public ff::Object {
public:
    static const ff::ObjectType Type;
    const ff::ObjectType& type() const override { return Type; };

    TilePlatform(ff::ActorInit init, ff::ObjectLevelData& data);

    void update(ff::World& w, secs deltaTime) override;
    void notify_level_reloaded(ff::World& w, const ff::Level& lvl);

private:
    ff::ID<ff::TileLayer> tl_id;
};

