#pragma once

#include "fastfall/game/level/TileLogic.hpp"

#include "fastfall/util/log.hpp"

using namespace ff;

class AnimLogic : public TileLogic {
public:
	AnimLogic(GameContext context) : TileLogic(context, "anim") {}

	void addTile(Vec2u tilePos, Tile tile, std::string arg) override;
	void removeTile(Vec2u tilePos) override;
	void update(secs deltaTime) override;

private:
	struct TileTimer {
		secs time_to_anim;
		Vec2u tile_impacted;
		Tile tile;
		bool discard = false;
	};

	std::vector<TileTimer> tile_timers;

};