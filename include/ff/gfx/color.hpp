#pragma once

#include <cstdint>

#include "glm/glm.hpp"

namespace ff {

struct color : glm::u8vec4 {

	constexpr color()
		: color(0, 0, 0, 255u)
	{

	}
	constexpr color(uint8_t t_red, uint8_t t_green, uint8_t t_blue, uint8_t t_alpha = 255)
		: glm::u8vec4{ t_red, t_green, t_blue, t_alpha }
	{
	}

	constexpr color(uint32_t t_value)
	{
		a = (t_value >> 0) & 0xFF;
		b = (t_value >> 8) & 0xFF;
		g = (t_value >> 16) & 0xFF;
		r = (t_value >> 24) & 0xFF;
	}

    constexpr inline color& set_r(uint8_t t_amount) { r = t_amount; return *this; };
    constexpr inline color& set_g(uint8_t t_amount) { g = t_amount; return *this; };
    constexpr inline color& set_b(uint8_t t_amount) { b = t_amount; return *this; };
    constexpr inline color& set_a(uint8_t t_amount) { a = t_amount; return *this; };

    constexpr uint32_t hex() const {
        return (r << 24) + (g << 16) + (b << 8) + a;
    }

    constexpr color operator() () const { return *this; };

    constexpr inline bool operator== (const color& t_color) const {
		return hex() == t_color.hex();
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