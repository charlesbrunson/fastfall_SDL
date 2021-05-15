
#include "Player.hpp"

AnimID Player::anim_idle     = AnimID::NONE;
AnimID Player::anim_idle_run = AnimID::NONE;
AnimID Player::anim_run      = AnimID::NONE;
AnimID Player::anim_jump     = AnimID::NONE;
AnimID Player::anim_air      = AnimID::NONE;
AnimID Player::anim_fall     = AnimID::NONE;

GameObjectLibrary::Entry<Player> plr_type;