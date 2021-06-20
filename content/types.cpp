
#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/level/TileLogic.hpp"
#include "fastfall/game/level/Tile.hpp"

#include "tilelogic/AnimLogic.hpp"

#include "object/Player.hpp"
#include "object/BasicPlatform.hpp"


using namespace ff;

bool isInit = false;

void game_InitTypes() {
	if (isInit) {
		return;
	}
	isInit = true;


	// tile materials

	Tile::addMaterialType({
			.typeName = "conveyor_slow",
			.surfaces = {
				SurfaceMaterial{.velocity = 150.f }, // north
				SurfaceMaterial{.velocity = 0.f },	// south
				SurfaceMaterial{.velocity = 40.f },	// east
				SurfaceMaterial{.velocity = 0.f }	// west
			}
		});

	Tile::addMaterialType({
			.typeName = "conveyor_slow_reverse",
			.surfaces = {
				SurfaceMaterial{.velocity = -150.f }, // north
				SurfaceMaterial{.velocity = 0.f },	// south
				SurfaceMaterial{.velocity = -40.f },	// east
				SurfaceMaterial{.velocity = 0.f }	// west
			}
		});

	// tile logics

	TileLogic::addType<AnimLogic>("anim");


	// objects

	GameObjectLibrary::addType<Player>({
			.typeName = "Player",
			.tile_size = { 1u, 2u },
			.properties = {
				ObjectTypeProperty("anotherprop", ObjectPropertyType::String),
				ObjectTypeProperty("faceleft", false)
			}
		});

	GameObjectLibrary::addType<BasicPlatform>({
			.typeName = "BasicPlatform",
			.tile_size = {0, 0},
			.properties = {
				ObjectTypeProperty("acceleration", ObjectPropertyType::Float),
				ObjectTypeProperty("max_velocity", ObjectPropertyType::Float),
				ObjectTypeProperty("path", object_null)
			} 
		});

}