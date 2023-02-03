
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
	ObjectFactory::register_object<Player>({
        .type                = { "Player" },
        .allow_as_level_data = true,
        .anim                = AnimIDRef{"player.sax", "idle"},
        .tile_size           = { 1u, 2u },
        .priority            = ActorPriority::Highest,
        .group_tags          = { "player" },
        .properties          = {
               { "faceleft",	 false },
               { "anotherprop", ObjectPropertyType::String }
        }
    });

	ObjectFactory::register_object<BasicPlatform>({
        .type = { "BasicPlatform" },
        .allow_as_level_data = true,
        .anim = std::nullopt,
        .tile_size = {0, 0},
        .group_tags = {	"platform" },
        .properties = {
              { "path",  ObjLevelID{ ObjLevelID::NO_ID } }
        }
    });

    ObjectFactory::register_object<JetPlatform>({
        .type       = { "JetPlatform" },
        .allow_as_level_data = true,
        .anim       = std::nullopt,
        .tile_size  = {0, 1},
        .group_tags = {	"platform" },
        .properties = {
            { "path",  ObjLevelID{ ObjLevelID::NO_ID } }
        }
    });

	ObjectFactory::register_object<SimpleEffect>({
        .type = { "SimpleEffect" }
    });

    ObjectFactory::register_object<TilePlatform>({
        .type = { "TilePlatform" },
        .allow_as_level_data = true,
        .anim = std::nullopt,
        .tile_size = { 0u, 0u },
        .group_tags = {	"platform" },
        .properties = {
            { "layer",	 ObjectPropertyType::Int },
            { "path",  ObjLevelID{ ObjLevelID::NO_ID } }
        }
    });
}