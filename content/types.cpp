
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

	Tile::addMaterial(
		TileMaterial{
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

	GameObjectLibrary::addType<Player>(
		ObjectType{
			.type = { "Player" },
			.anim = std::make_optional(AnimIDRef{"player", "idle"}),
			.tile_size = { 1u, 2u },
			.group_tags = {
				"player"
			},
			.properties = {
				ObjectProperty{"anotherprop", ObjectPropertyType::String},
				ObjectProperty{"faceleft", false}
			}
		});

	GameObjectLibrary::addType<BasicPlatform>(
		ObjectType{
			.type = { "BasicPlatform" },
			.anim = std::nullopt,
			.tile_size = {0, 0},
			.group_tags = {
				"platform"
			},
			.properties = {
				ObjectProperty{"acceleration", ObjectPropertyType::Float},
				ObjectProperty{"max_velocity", ObjectPropertyType::Float},
				ObjectProperty{"path", object_null}
			} 
		});

}