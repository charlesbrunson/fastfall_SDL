#pragma once

#include "fastfall/game_v2/object/GameObject.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/engine/time/time.hpp"
#include"fastfall/util/id.hpp"

#include <list>
#include <map>

#include "fastfall/render/RenderTarget.hpp"

namespace ff {

class World;

class ObjectSystem {
public:
	void update(secs deltaTime);
	void predraw(float interp, bool updated);

    inline void set_world(World* w) { world = w; }
    void notify_created(ID<GameObject> id);
    void notify_erased(ID<GameObject> id);

private:
    World* world;
};

}