#pragma once

#include <cstdint>

#include "glm/glm.hpp"

namespace ff {

struct Color {

	constexpr Color() 
		: Color(0, 0, 0, 255u)
	{

	}
	constexpr Color(uint8_t red, uint8_t green, uint8_t blue) 
		: Color(red, green, blue, 255u)
	{
	}

	constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
		: r(red)
        , g(green)
        , b(blue)
        , a(alpha)
	{
	}

	constexpr Color(uint32_t value)
	{
		a = (value >> 0) & 0xFF;
		b = (value >> 8) & 0xFF;
		g = (value >> 16) & 0xFF;
		r = (value >> 24) & 0xFF;
	}

    constexpr inline Color& red  (uint8_t amount) { r = amount; return *this; };
    constexpr inline Color& green(uint8_t amount) { g = amount; return *this; };
    constexpr inline Color& blue (uint8_t amount) { b = amount; return *this; };
    constexpr inline Color& alpha(uint8_t amount) { a = amount; return *this; };

    constexpr uint32_t hex() const {
        return (r << 24) + (g << 16) + (b << 8) + a;
    }

    constexpr Color operator() () const { return *this; };

    constexpr inline bool operator== (const Color& color) const {
		return hex() == color.hex();
	}

    static const Color Transparent;
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

constexpr inline Color Color::Transparent  { 0x00000000 };
constexpr inline Color Color::White        { 0xFFFFFFFF };
constexpr inline Color Color::Black        { 0x000000FF };
constexpr inline Color Color::Red          { 0xFF0000FF };
constexpr inline Color Color::Green        { 0x00FF00FF };
constexpr inline Color Color::Blue         { 0x0000FFFF };
constexpr inline Color Color::Yellow       { 0xFFFF00FF };
constexpr inline Color Color::Cyan         { 0x00FFFFFF };
constexpr inline Color Color::Magenta      { 0xFF00FFFF };

}