#pragma once

#include <string_view>
#include <array>

#include "fastfall/render/freetype.hpp"
#include "Texture.hpp"

namespace ff {

class Font
{
public:
	struct GlyphMetrics
	{
		glm::ivec2 size;
		glm::ivec2 bearing;
		long advance_x;
	};

public:

	Font();

	bool loadFromFile(std::string_view font_file, unsigned pixel_size);
	bool loadFromStream(const void* font_data, short length, unsigned pixel_size);

	const Texture& getBitmapTex() const { return font_bitmap; };
	const GlyphMetrics& getMetrics(unsigned char ch) const { return glyph_metrics[ch]; };

	constexpr static uint8_t CHAR_COUNT = 128;

private:

	bool load(FT_Face face);

	Texture font_bitmap;
	glm::i64vec2 glyph_max_size;
	std::array<GlyphMetrics, CHAR_COUNT> glyph_metrics;
	bool loaded = false;
};

}