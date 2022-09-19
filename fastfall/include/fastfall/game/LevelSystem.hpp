#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/game/level/Level.hpp"

namespace ff {

class World;

class LevelSystem {
public:
    inline void set_world(World* w) { world = w; }

    void notify_created(ID<Level> id);
    void notify_erased(ID<Level> id);

    std::optional<ID<Level>> get_active_id() const;
    Level* get_active() const;
    void set_active(ID<Level> id);

private:
    std::optional<ID<Level>> active_level;
    World* world;
};

}
