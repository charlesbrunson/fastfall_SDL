#pragma once

#include <string_view>

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
	explicit constexpr AnimID(unsigned int id) :
		value{ id }
	{
	};

	unsigned int value;
	static unsigned int counter;
};

class AnimIDRef {
public:
	AnimIDRef() {};
	AnimIDRef(std::string_view sprite, std::string_view anim);

	AnimIDRef(const AnimIDRef&) = default;
	AnimIDRef& operator=(const AnimIDRef&) = default;

	AnimIDRef(AnimIDRef&&) = default;
	AnimIDRef& operator=(AnimIDRef&&) = default;

	AnimID id() const;

	operator AnimID() const {
		return id();
	}

private:
	mutable AnimID m_id = AnimID::NONE;

	std::string_view m_sprite;
	std::string_view m_anim;
};

}
