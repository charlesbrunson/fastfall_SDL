#pragma once

#include "fastfall/game/ID.hpp"
#include "fastfall/game/Entity.hpp"
//#include "fastfall/game/WorldState.hpp"

namespace ff {

class WorldState;

class WorldStateListener
{
public:
	virtual ~WorldStateListener() = default;

	virtual void notify_component_created(WorldState& st, ID<Entity> entity, GenericID gid) {};
	virtual void notify_component_destroyed(WorldState& st, ID<Entity> entity, GenericID gid) {};
};

}