#pragma once

#include <string_view>
#include <array>

#include "fastfall/util/Vec2.hpp"
#include "fastfall/render/freetype.hpp"
#include "Texture.hpp"

namespace ff {

class Font
{
public:
	struct GlyphMetrics
	{
		unsigned glyph_index;
		glm::ivec2 size		= { 0,0 };
		glm::ivec2 bearing	= { 0,0 };
		unsigned advance_x	= 0;
	};

public:

	Font();
	~Font();

	bool loadFromFile(std::string_view font_file, unsigned pixel_size);
	bool loadFromStream(const void* font_data, short length, unsigned pixel_size);

	const Texture& getBitmapTex() const { return font_bitmap; };
	const GlyphMetrics& getMetrics(unsigned char ch) const { return glyph_metrics[ch]; };
	glm::i64vec2 getGlyphSize() const { return glyph_max_size; };
	unsigned getPixelSize() const { return px_size; };

	int getYMin() const { return yMin; };
	int getYMax() const { return yMax; };
	int getHeight() const { return height; };

	Vec2i getKerning(char left, char right) const;

	bool isLoaded() const { return m_face != nullptr; }

	constexpr static uint8_t CHAR_COUNT = 128;

private:

	bool load(FT_Face face);

	int yMin			= 0;
	int yMax			= 0;
	int height			= 0;
	unsigned px_size	= 0;

	Texture font_bitmap;
	glm::i64vec2 glyph_max_size;
	std::array<GlyphMetrics, CHAR_COUNT> glyph_metrics;
	FT_Face m_face = nullptr;
};

}