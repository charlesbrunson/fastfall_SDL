#pragma once

#include "fastfall/util/math.hpp"
#include <cstdint>
#include <concepts>

namespace ff {

struct TileID {
	enum TileID_Mask : uint16_t
	{
		PadTop		= 0b1000'0000'0000'0000,
		PadRight	= 0b0100'0000'0000'0000,
		PadBottom	= 0b0010'0000'0000'0000,
		PadLeft		= 0b0001'0000'0000'0000,

		PadMask = 0b1111'0000'0000'0000,

		Y = 0b0000'1111'1100'0000,
		X = 0b0000'0000'0011'1111,

		InvalidTile = ( Y | X )
	};

	static constexpr unsigned dimension_max = 64u;

	uint16_t value = TileID_Mask::InvalidTile;

	TileID() = default;

	constexpr TileID(
		std::unsigned_integral auto _x,
		std::unsigned_integral auto _y)
	{
		if (_x >= dimension_max || _y >= dimension_max) {
			reset();
		}
		else {
			setX(_x);
			setY(_y);
		}
	}

	explicit constexpr TileID(Vec2u v)
		: TileID(v.x, v.y)
	{
	}

	constexpr uint8_t getX() const
	{
		return (value & TileID_Mask::X);
	}

	constexpr uint8_t getY() const
	{
		return (value & TileID_Mask::Y) >> 6;
	}

	constexpr void setX(uint8_t _x)
	{
		value = (value & ~(TileID_Mask::X)) | ( _x & TileID_Mask::X);
	}

	constexpr void setY(uint8_t _y)
	{
		value = (value & ~(TileID_Mask::Y)) | ((_y << 6) & TileID_Mask::Y);
	}

	constexpr uint8_t getPadding() const
	{
		return (value & TileID_Mask::PadMask) >> 12;
	}

	constexpr void setPadding(unsigned cardinal_bits)
	{
		value = (value & ~(TileID_Mask::PadMask)) | (cardinal_bits << 12);
	}

	constexpr void setPadding(Cardinal dir, bool set)
	{
		unsigned pos = (15u - (unsigned)dir);

		value = (value & ~(1 << pos)) | (set << pos);
	}

	constexpr bool hasPadding(Cardinal dir) const
	{
		return (value & direction::to_bits(dir)) > 0;
	}

	constexpr Vec2u to_vec() const {
		return Vec2u{ getX(), getY() };
	}

	constexpr void reset()
	{
		value = TileID_Mask::InvalidTile;
	}

	constexpr bool valid() const {
		return (value & TileID_Mask::InvalidTile) != TileID_Mask::InvalidTile;
	}

	/*
	constexpr operator bool() const {
		return valid();
	}
	*/

	/*
	constexpr operator Vec2u() const {
		return to_vec();
	}
	*/

	constexpr bool operator== (TileID id) const {
		return getX() == id.getX() && getY() == id.getY();
	}

	constexpr bool operator!= (TileID id) const {
		return !(*this == id);
	}

	constexpr TileID operator+ (Vec2u rhs) const {
		TileID r = *this;
		r.setPadding(this->getPadding());
		r.setX((r.getX() + rhs.x) & 63u);
		r.setY((r.getY() + rhs.y) & 63u);
		return r;
	}

};

}