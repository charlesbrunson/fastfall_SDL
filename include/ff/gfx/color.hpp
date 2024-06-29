#pragma once

#include <cstdint>

#include "ff/util/math.hpp"

namespace ff {

struct color : u8vec4 {

	constexpr color()
		: color(0u, 0u, 0u, 255u)
	{
	}

	constexpr color(uint8_t t_red, uint8_t t_green, uint8_t t_blue, uint8_t t_alpha = 255)
		: u8vec4{ t_red, t_green, t_blue, t_alpha }
	{
	}

	constexpr color(uint32_t t_value)
	{
		a = (t_value >> 0) & 0xFF;
		b = (t_value >> 8) & 0xFF;
		g = (t_value >> 16) & 0xFF;
		r = (t_value >> 24) & 0xFF;
	}

    inline static color from_floats(f32 t_red, f32 t_green, f32 t_blue, f32 t_alpha = 1.f) {
        return color{ packUnorm4x8( vec4{t_alpha, t_blue, t_green, t_red } ) };
    }

    inline static color from_floats(vec4 values) {
        return from_floats(values.x, values.y, values.z, values.w);
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