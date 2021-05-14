#include "fastfall/resource/asset/AnimAsset.hpp"
#include "fastfall/resource/asset/SpriteAsset.hpp"

namespace ff {

AnimID AnimID::NONE = AnimID{ 0 };
unsigned int AnimID::counter = 0;

Animation::Animation(const SpriteAsset* sprite, AnimID my_id) :
	my_sprite(sprite),
	anim_id(my_id)
{
}

const std::string_view Animation::get_sprite_name() const noexcept {
	return my_sprite->getAssetName();
}

const Texture& Animation::get_sprite_texture() const noexcept {
	return my_sprite->getTexture();
}

}
