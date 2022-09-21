#pragma once

#include "fastfall/game/object/GameObject.hpp"

//#include "fastfall/render/Drawable.hpp"
//#include "fastfall/render/RenderTarget.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/id.hpp"

#include <list>
#include <map>


namespace ff {

class World;

class ObjectSystem {
public:
	void update(World& world, secs deltaTime);
	void predraw(World& world, float interp, bool updated);

    //inline void set_world(World* w) { world = w; }
    void notify_created(World& world, ID<GameObject> id) {}
    void notify_erased(World& world, ID<GameObject> id) {}

private:
    //World* world;
};

}
