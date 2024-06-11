#pragma once

#include <cstdint>

#include "glm/glm.hpp"

namespace ff {

struct color : glm::vec<4, uint8_t> {

	constexpr color()
		: color(0, 0, 0, 255u)
	{

	}
	constexpr color(uint8_t red, uint8_t green, uint8_t blue)
		: color(red, green, blue, 255u)
	{
	}

	constexpr color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
		: glm::vec<4, uint8_t>{ red, green, blue, alpha }
	{
	}

	constexpr color(uint32_t value)
	{
		a = (value >> 0) & 0xFF;
		b = (value >> 8) & 0xFF;
		g = (value >> 16) & 0xFF;
		r = (value >> 24) & 0xFF;
	}

    constexpr inline color& set_r(uint8_t amount) { r = amount; return *this; };
    constexpr inline color& set_g(uint8_t amount) { g = amount; return *this; };
    constexpr inline color& set_b(uint8_t amount) { b = amount; return *this; };
    constexpr inline color& set_a(uint8_t amount) { a = amount; return *this; };

    constexpr uint32_t hex() const {
        return (r << 24) + (g << 16) + (b << 8) + a;
    }

    constexpr color operator() () const { return *this; };

    constexpr inline bool operator== (const color& color) const {
		return hex() == color.hex();
	}

    static const color transparent;
    static const color white;
    static const color black;
    static const color red;
    static const color green;
    static const color blue;
    static const color yellow;
    static const color cyan;
    static const color magenta;
};

constexpr inline color color::transparent  { 0x00000000 };
constexpr inline color color::white        { 0xFFFFFFFF };
constexpr inline color color::black        { 0x000000FF };
constexpr inline color color::red          { 0xFF0000FF };
constexpr inline color color::green        { 0x00FF00FF };
constexpr inline color color::blue         { 0x0000FFFF };
constexpr inline color color::yellow       { 0xFFFF00FF };
constexpr inline color color::cyan         { 0x00FFFFFF };
constexpr inline color color::magenta      { 0xFF00FFFF };

}