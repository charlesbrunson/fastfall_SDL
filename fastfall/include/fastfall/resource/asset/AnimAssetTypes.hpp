#pragma once

#include <string_view>

namespace ff {

class AnimID {
public:
	constexpr unsigned int get() const noexcept {
		return value;
	}

	constexpr unsigned int operator() () const noexcept {
		return value;
	}

    constexpr AnimID() = default;

	constexpr AnimID(const AnimID& id) {
		value = id.value;
	}

	static AnimID reserve_id() {
		counter++;
		return AnimID{ counter };
	}

	constexpr bool operator< (const AnimID& id) const noexcept {
		return value < id.value;
	}

	constexpr bool operator== (const AnimID& id) const noexcept {
		return value == id.value;
	}

	constexpr void operator= (const AnimID& id) {
		value = id.value;
	}

	static void resetCounter() {
		counter = 0;
	}

private:
	explicit constexpr AnimID(unsigned int id) :
		value{ id }
	{
	};

	unsigned int value = 0;
	static unsigned int counter;
};

class AnimIDRef {
public:
	constexpr AnimIDRef() = default;
    constexpr AnimIDRef(std::string_view sprite, std::string_view anim)
        : m_sprite(sprite), m_anim(anim)
    {
    }

    constexpr AnimIDRef(const AnimIDRef&) = default;
    AnimIDRef& operator=(const AnimIDRef&) = default;

    constexpr AnimIDRef(AnimIDRef&&) = default;
    AnimIDRef& operator=(AnimIDRef&&) = default;

    AnimID id() const;

    operator AnimID() const {
		return id();
	}

private:
	mutable AnimID m_id = {};

	std::string_view m_sprite;
	std::string_view m_anim;
};

}
