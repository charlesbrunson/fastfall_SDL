
#include "Player.hpp"

AnimIDRef Player::anim_idle("player", "idle");
AnimIDRef Player::anim_land("player", "land");
AnimIDRef Player::anim_run ("player", "running");
AnimIDRef Player::anim_jump("player", "jump");
AnimIDRef Player::anim_fall("player", "fall");

GameObjectLibrary::Entry<Player> plr_type{ 
	{
		.tile_size = {1, 2},
		.properties = {
			ObjectTypeProperty("anotherprop", ObjectPropertyType::String),
			ObjectTypeProperty("faceleft", false)
		}
	} 
};