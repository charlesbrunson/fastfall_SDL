
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

void register_types()
{
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

    // engine states
    // ff::user_types::register_engine_state<TestState>("Test State");

	// tile logic
	TileLogic::addType<AnimLogic>("anim");
    // ff::user_types::register_tile_logic<AnimLogic>("anim");

	// objects
    // ff::user_types::register_actor<Player>();
    // ff::user_types::register_actor<BasicPlatform>();
    // ff::user_types::register_actor<JetPlatform>();
    // ff::user_types::register_actor<SimpleEffect>();
    // ff::user_types::register_actor<TilePlatform>();
}