#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/game/level/Level.hpp"

namespace ff {

class World;

class LevelSystem {
public:
    void update(World& world, secs deltaTime);
    void predraw(World& world, float interp, bool updated);

    void notify_created(World& world, ID<Level> id);
    void notify_erased(World& world, ID<Level> id);

    std::optional<ID<Level>> get_active_id() const;
    Level* get_active(World& world) const;
    void set_active(ID<Level> id);

private:
    std::optional<ID<Level>> active_level;
};

}
