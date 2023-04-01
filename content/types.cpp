
#include "object/Player.hpp"
#include "object/BasicPlatform.hpp"
#include "object/JetPlatform.hpp"
#include "object/SimpleEffect.hpp"
#include "object/TilePlatform.hpp"
#include "tilelogic/AnimLogic.hpp"
#include "TestState.hpp"

#include "fastfall/user_types.hpp"

using namespace ff;

void register_types()
{
    // engine states
    ff::user_types::register_engine_state<TestState>("teststate");

    // objects
    ff::user_types::register_actor<Player>();
    ff::user_types::register_actor<BasicPlatform>();
    ff::user_types::register_actor<JetPlatform>();
    //ff::user_types::register_actor<SimpleEffect>();
    ff::user_types::register_actor<TilePlatform>();

    // tile logic
    ff::user_types::register_tile_logic<AnimLogic>("anim");

	// tile materials - this could be an asset file?
    ff::user_types::register_tile_material({
			.typeName = "conveyor_slow",
			.surfaces = {
				SurfaceMaterial{ .velocity = 120.f }, // north
				SurfaceMaterial{ .velocity = 0.f },	// south
				SurfaceMaterial{ .velocity = 120.f },	// east
				SurfaceMaterial{ .velocity = 0.f }	// west
			}
		});

    ff::user_types::register_tile_material({
            .typeName = "conveyor_slow_reverse",
            .surfaces = {
                SurfaceMaterial{ .velocity = -120.f }, // north
                SurfaceMaterial{ .velocity = 0.f },	// south
                SurfaceMaterial{ .velocity = -120.f },	// east
                SurfaceMaterial{ .velocity = 0.f }	// west
            }
        });
}