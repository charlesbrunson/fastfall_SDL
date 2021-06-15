#pragma once

#include "fastfall/game/level/TileLogic.hpp"

namespace ff {

class AnimLogic : public TileLogic {
public:
	AnimLogic(GameContext context)
		: TileLogic(context)
	{

	}

	void addTile(Vec2u tilePos, std::string arg) override;
	void updateLogic(secs deltaTime) override;

};



}
