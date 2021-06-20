
#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/level/TileLogic.hpp"

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


	// tile logics

	TileLogic::addType<AnimLogic>("anim");


	// objects

	GameObjectLibrary::addType<Player>({
		.typeName = "Player",
		.tile_size = { 0u, 0u },
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