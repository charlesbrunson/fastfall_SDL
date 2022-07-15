#pragma once

#include "fastfall/game/WorldStateListener.hpp"
#include "fastfall/game/WorldState.hpp"

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

namespace ff {

class CollidableSystem : public WorldStateListener
{
public:
	void precollision_update(WorldState& st, secs deltaTime);

};

}
