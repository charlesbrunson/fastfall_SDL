#pragma once

#include "InstanceID.hpp"

//class GameInstance;

// lightweight containing interfaces to all the game instance's managers
// meant to be passed to lower game entities and the like

namespace ff {

class GameContext {
private:

	GameContext();
	GameContext(InstanceID instanceID);

	friend class GameInstance;
	friend class InstanceObserver;

	InstanceID id;

public:
	GameContext(const GameContext& context) = default;
	GameContext& operator=(const GameContext& context) = default;

	inline InstanceID getID() const noexcept { return id; };
	bool valid() const noexcept;
};

}