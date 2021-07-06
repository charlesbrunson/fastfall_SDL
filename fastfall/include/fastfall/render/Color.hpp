#pragma once

#include <cstdint>

#include <glm/glm.hpp>

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
		:	r(red),
			g(green),
			b(blue),
			a(alpha)
	{

	}
	constexpr Color(uint32_t value)
	{
		a = (value >> 0) & 0xFF;
		b = (value >> 8) & 0xFF;
		g = (value >> 16) & 0xFF;
		r = (value >> 24) & 0xFF;
	}

	glm::uvec4 toVec4() const;

	inline Color& red  (uint8_t amount) { r = amount; return *this; };
	inline Color& green(uint8_t amount) { g = amount; return *this; };
	inline Color& blue (uint8_t amount) { b = amount; return *this; };
	inline Color& alpha(uint8_t amount) { a = amount; return *this; };

	uint32_t hex() const;

	static const Color Transparent;
	static const Color White;
	static const Color Black;
	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Cyan;
	static const Color Magenta;

	Color operator() () const { return *this; };

	inline bool operator== (const Color& color) const {
		return hex() == color.hex();
	}

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

};

}