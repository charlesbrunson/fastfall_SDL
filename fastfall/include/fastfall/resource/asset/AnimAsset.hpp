#pragma once

#include "fastfall/util/math.hpp"
//#include "SFML/Graphics.hpp"

#include "fastfall/render/util/Texture.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include <assert.h>
#include <map>

namespace ff {

class SpriteAsset;

class Animation {
public:

	Animation(const SpriteAsset* sprite = nullptr, AnimID my_id = {});

	void operator= (const Animation& anim) noexcept {
		anim_id = anim.anim_id;

		anim_name = anim.anim_name;
		area = anim.area;
		origin = anim.origin;
		loop = anim.loop;
		framerateMS = anim.framerateMS;
		chain = anim.chain;
        offsets = anim.offsets;

		my_sprite = anim.my_sprite;
	}
	void operator= (Animation&& anim) noexcept {
		anim_id = anim.anim_id;

		anim_name = anim.anim_name;
		area = anim.area;
		origin = anim.origin;
		loop = anim.loop;
		framerateMS = anim.framerateMS;
		chain = anim.chain;
        offsets = std::move(anim.offsets);

		my_sprite = anim.my_sprite;

	};

	const std::string_view get_sprite_name() const noexcept;
	const Texture& get_sprite_texture() const noexcept;

	std::string anim_name;
	AnimID anim_id;

	Recti area = Recti(0, 0, 0, 0);
	Vec2i origin = Vec2i(0, 0);
	unsigned loop = 0;
	std::vector<unsigned> framerateMS;

    std::map<std::string, Vec2f, std::less<>> offsets;

    std::optional<Vec2f> get_offset(std::string_view str, bool hflip = false) const {
        if (auto it = offsets.find(str); it != offsets.end()) {
            return Vec2f{ it->second.x * (hflip ? -1.f : 1.f), it->second.y };
        }
        else {
            return {};
        }
    }

	struct AnimChain {
		AnimID anim_id = {};
		unsigned int start_frame = 0;
		bool has_chain = false;
	} chain;

private:
	const SpriteAsset* my_sprite;
};


}