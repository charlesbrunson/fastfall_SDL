#pragma once

#include "fastfall/util/math.hpp"
#include <cstdint>
#include <concepts>

namespace ff {

struct TileID {
	uint8_t y : 4 = 15u;
	uint8_t x : 4 = 15u;

	TileID() = default;
	constexpr TileID(
		std::unsigned_integral auto _x,
		std::unsigned_integral auto _y)
	{
		if (_x > 15u || _y > 15u) {
			reset();
		}
		else {
			x = _x;
			y = _y;
		}
	}

	explicit constexpr TileID(Vec2u v) {
		if (v.x > 15u || v.y > 15u) {
			reset();
		}
		else {
			x = v.x;
			y = v.y;
		}
	}

	explicit constexpr TileID(uint8_t id) {
		*this = std::bit_cast<uint8_t>(id);
	}

	constexpr TileID& operator= (uint8_t id) {
		y = id >> 4;
		x = id & 15u;
		return *this;
	}

	constexpr Vec2u to_vec() const {
		return Vec2u{ x, y };
	}
	constexpr uint8_t to_byte() const {
		return std::bit_cast<uint8_t>(*this);
	}

	constexpr void reset() {
		*this = std::bit_cast<uint8_t>(UINT8_MAX);
	}

	constexpr bool valid() const {
		return std::bit_cast<uint8_t>(*this) != UINT8_MAX;
	}

	constexpr operator bool() const {
		return std::bit_cast<uint8_t>(*this) != UINT8_MAX;
	}
	constexpr operator Vec2u() const {
		return to_vec();
	}
	constexpr operator uint8_t() const {
		return to_byte();
	}

	constexpr bool operator== (TileID id) const {
		return std::bit_cast<uint8_t>(id) == std::bit_cast<uint8_t>(*this);
	}
	constexpr bool operator!= (TileID id) const {
		return std::bit_cast<uint8_t>(id) != std::bit_cast<uint8_t>(*this);
	}

	constexpr TileID operator+ (TileID rhs) {
		TileID r = *this;
		r.x += rhs.x;
		r.y += rhs.y;
		return r;
	}

};

}