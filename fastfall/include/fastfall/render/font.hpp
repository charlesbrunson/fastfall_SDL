#pragma once

#include <string_view>
#include <array>
#include <deque>

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

	bool loadFromFile(std::string_view font_file);
	bool loadFromStream(const void* font_data, short length);

	void unload();

	bool cachePixelSize(unsigned pixel_size);
	bool setPixelSize(unsigned pixel_size);

	const Texture& getBitmapTex() const { return curr_cache->bitmap_texture; };
	void loadBitmapTex(unsigned px_size) const;

	//const SDL_Surface*  getBitmapSurface() const { return curr_cache->font_bitmap_surface; };

	const GlyphMetrics& getMetrics(unsigned char ch) const { return curr_cache->glyph_metrics[ch]; };
	glm::i64vec2		getGlyphSize() const { return curr_cache->glyph_max_size; };
	unsigned			getPixelSize() const { return curr_cache->px_size; };

	int getYMin()	const { return curr_cache->yMin; };
	int getYMax()	const { return curr_cache->yMax; };
	int getHeight() const { return curr_cache->height; };

	//Vec2i getKerning(char left, char right) const;

	bool isLoaded() const { return m_face != nullptr; }

	constexpr static uint8_t CHAR_COUNT = 128;

private:
	struct FontCache
	{
		bool valid = false;

		int yMin = 0;
		int yMax = 0;
		int height = 0;
		unsigned px_size = 0;

		SDL_Surface* bitmap_surface = nullptr;
		Texture bitmap_texture;

		glm::i64vec2 glyph_max_size;
		std::array<GlyphMetrics, CHAR_COUNT> glyph_metrics;
	};

	FontCache* cache_for_size(unsigned size);

	std::vector<std::unique_ptr<FontCache>> caches;
	FontCache* curr_cache = nullptr;

	FT_Face m_face = nullptr;
};

}