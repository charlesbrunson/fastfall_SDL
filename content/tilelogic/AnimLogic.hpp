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

	/*
	bool on_precontact(Vec2i tilePos, const Contact& contact, secs duration) const override {

		LOG_INFO("A: {}", tilePos.to_string());
		return true;
	};

	void on_postcontact(Vec2i tilePos, const PersistantContact& contact) const override {

		LOG_INFO("B: {}", tilePos.to_string());
	};
	*/


private:
	struct TileTimer {
		secs time_to_anim;
		Vec2u tile_impacted;
		Tile tile;
	};

	std::vector<TileTimer> tile_timers;

};