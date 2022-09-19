#include "fastfall/game/ObjectSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

void ObjectSystem::update(secs deltaTime) {
    for (auto& obj : world->all<GameObject>()) {
        obj->update(deltaTime);
    }
}

void ObjectSystem::predraw(float interp, bool updated) {
    for (auto& obj : world->all<GameObject>()) {
        obj->predraw(interp, updated);
    }
}

}
