#pragma once
#include <optional>

#include "fastfall/util/Vec2.hpp"
#include "fastfall/util/Angle.hpp"

#include <vector>
#include <array>
#include <string>

namespace ff {

enum Cardinal : uint8_t {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3
};

enum Ordinal : uint8_t {
	NORTHWEST = 0,
	NORTHEAST = 1,
	SOUTHEAST = 2,
	SOUTHWEST = 3
};

constexpr unsigned cardinalBit[4] = {
	1 << Cardinal::NORTH,
	1 << Cardinal::EAST,
	1 << Cardinal::SOUTH,
	1 << Cardinal::WEST,
};

constexpr Cardinal CARDINAL_FIRST = NORTH;
constexpr Cardinal CARDINAL_LAST = WEST;

constexpr Vec2f cardinalToVec2f(Cardinal direction) {
	constexpr std::array<Vec2f, 4u> unit = {
		Vec2f{0.f, -1.f},
		Vec2f{1.f, 0.f},
		Vec2f{0.f, 1.f},
		Vec2f{-1.f, 0.f}
	};
	return unit[static_cast<unsigned>(direction)];
}
constexpr Vec2i cardinalToVec2i(Cardinal direction) {
	constexpr std::array<Vec2i, 4u> unit = {
		Vec2i{0, -1},
		Vec2i{1, 0},
		Vec2i{0, 1},
		Vec2i{-1, 0}
	};
	return unit[static_cast<unsigned>(direction)];
}
constexpr Angle cardinalToAngle(Cardinal direction) {
	std::array<Angle, 4u> angle = {
		Angle{-90.f},
		Angle{0.f},
		Angle{90.f},
		Angle{180.f}
	};
	return angle[static_cast<unsigned>(direction)];
}


template<class T>
constexpr std::optional<Cardinal> vecToCardinal(Vec2<T> v) {
	if (v.y == 0.f) {
		if (v.x > 0) {
			return Cardinal::EAST;
		}
		else if (v.x < 0) {
			return Cardinal::WEST;
		}
	}
	else if (v.x == 0.f) {
		if (v.y > 0) {
			return Cardinal::SOUTH;
		}
		else if (v.y < 0) {
			return Cardinal::NORTH;
		}
	}
	return std::nullopt;
}


Cardinal cardinalOpposite(Cardinal direction) noexcept;

std::vector<Cardinal> cardinalFromBits(unsigned cardinalBits) noexcept;

std::string cardinalToString(Cardinal direction) noexcept;

constexpr unsigned cardinalToBits(Cardinal direction) noexcept {
	unsigned r = 0u;
	switch (direction) {
	case Cardinal::NORTH:	r = cardinalBit[Cardinal::NORTH]; break;
	case Cardinal::EAST:	r = cardinalBit[Cardinal::EAST];  break;
	case Cardinal::SOUTH:	r = cardinalBit[Cardinal::SOUTH]; break;
	case Cardinal::WEST:	r = cardinalBit[Cardinal::WEST];  break;
	};
	return r;
};

template<class ...Args>
constexpr unsigned cardinalToBits(Cardinal direction, Args... args) noexcept {
	return cardinalToBits(direction) | cardinalToBits(args...);
};

}