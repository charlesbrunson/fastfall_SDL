#pragma once

#include "fastfall/util/math.hpp"
//#include "SFML/Graphics.hpp"

#include "fastfall/render/Texture.hpp"

#include <assert.h>

namespace ff {


class AnimID {
public:
	static AnimID NONE;

	constexpr unsigned int get() const noexcept {
		return value;
	}

	constexpr unsigned int operator() () const noexcept {
		return value;
	}

	constexpr AnimID(const AnimID& id) {
		value = id.value;
	}

	static AnimID reserve_id() {
		counter++;
		return AnimID{ counter };
	}

	bool operator< (const AnimID& id) const noexcept {
		return value < id.value;
	}

	bool operator== (const AnimID& id) const noexcept {
		return value == id.value;
	}

	void operator= (const AnimID& id) {
		value = id.value;
	}

	static void resetCounter() {
		counter = 0;
	}

private:
	constexpr AnimID(unsigned int id) :
		value{ id }
	{
	};

	unsigned int value;
	static unsigned int counter;
};

class AnimIDRef {
public:
	AnimIDRef(std::string_view sprite, std::string_view anim);

	AnimID id();

private:
	AnimID m_id = AnimID::NONE;

	std::string_view m_sprite;
	std::string_view m_anim;
};

class SpriteAsset;

class Animation {
public:

	Animation(const SpriteAsset* sprite = nullptr, AnimID my_id = AnimID::NONE);

	void operator= (const Animation& anim) noexcept {
		anim_id = anim.anim_id;

		anim_name = anim.anim_name;
		area = anim.area;
		origin = anim.origin;
		loop = anim.loop;
		framerateMS = anim.framerateMS;
		chain = anim.chain;

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

	struct AnimChain {
		AnimID anim_id = AnimID::NONE;
		unsigned int start_frame = 0;
		bool has_chain = false;
	} chain;

private:
	const SpriteAsset* my_sprite;
};

}