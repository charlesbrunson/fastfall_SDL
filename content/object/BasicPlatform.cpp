#include "BasicPlatform.hpp"

GameObjectLibrary::Entry<BasicPlatform> platform_type{
	{
		.tile_size = {0, 0},
		.properties = {
			ObjectTypeProperty("acceleration", ObjectPropertyType::Float),
			ObjectTypeProperty("max_velocity", ObjectPropertyType::Float),
			ObjectTypeProperty("path", object_null)
		}
	}
};
