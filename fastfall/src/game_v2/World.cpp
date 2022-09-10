#include "fastfall/game_v2/World.hpp"

namespace ff {

World::World() {
    _collision_system.set_world(this);
    _trigger_system.set_world(this);
    _camera_system.set_world(this);
    _scene_system.set_world(this);
    _object_system.set_world(this);
}


}