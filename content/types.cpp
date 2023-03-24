
#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/level/TileLogic.hpp"
#include "fastfall/game/tile/Tile.hpp"

#include "tilelogic/AnimLogic.hpp"

#include "object/Player.hpp"
#include "object/BasicPlatform.hpp"
#include "object/JetPlatform.hpp"
#include "object/SimpleEffect.hpp"
#include "object/TilePlatform.hpp"


using namespace ff;

bool isInit = false;

void game_InitTypes() {
	if (isInit) {
		return;
	}
	isInit = true;

	// tile materials - this could be an asset file?
	Tile::addMaterial(
		TileMaterial {
			.typeName = "conveyor_slow",
			.surfaces = {
				SurfaceMaterial{.velocity = 120.f }, // north
				SurfaceMaterial{.velocity = 0.f },	// south
				SurfaceMaterial{.velocity = 120.f },	// east
				SurfaceMaterial{.velocity = 0.f }	// west
			}
		});

	Tile::addMaterial(
		TileMaterial{
			.typeName = "conveyor_slow_reverse",
			.surfaces = {
				SurfaceMaterial{.velocity = -120.f }, // north
				SurfaceMaterial{.velocity = 0.f },	// south
				SurfaceMaterial{.velocity = -120.f },	// east
				SurfaceMaterial{.velocity = 0.f }	// west
			}
		});

	// tile logics
	TileLogic::addType<AnimLogic>("anim");

	// objects
	ObjectFactory::register_object<Player>();
	ObjectFactory::register_object<BasicPlatform>();
    ObjectFactory::register_object<JetPlatform>();
	ObjectFactory::register_object<SimpleEffect>();
    ObjectFactory::register_object<TilePlatform>();
}