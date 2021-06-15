#include "AnimLogic.hpp"

namespace ff {

TileLogicType type(
	"anim",
	[](GameContext context) -> std::unique_ptr<TileLogic> {
		return std::make_unique<AnimLogic>(context);
	}
);

void AnimLogic::addTile(Vec2u tilePos, std::string arg) {

}

void AnimLogic::updateLogic(secs deltaTime) {

}

}

