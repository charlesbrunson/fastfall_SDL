#pragma once

#include <string_view>
#include <array>
#include <deque>

#include "fastfall/util/glm_types.hpp"
#include "fastfall/render/external/freetype.hpp"
#include "Texture.hpp"

namespace ff {

class Font
{
public:
	struct GlyphMetrics
	{
		unsigned glyph_index = 0;
		Vec2i size		= { 0,0 };
		Vec2i bearing	= { 0,0 };
		unsigned advance_x	= 0;
	};

public:

	Font();
	Font(const Font&) = delete;
	Font(Font&&) = default;
	~Font();

	Font& operator=(const Font&) = delete;
	Font& operator=(Font&&) = default;

	bool loadFromFile(std::string_view font_file);
    bool loadFromFile(const std::filesystem::path& font_path);
	bool loadFromStream(const void* font_data, short length);

	void unload();

	bool cachePixelSize(unsigned pixel_size) const;
	bool setPixelSize(unsigned pixel_size) const;

	const Texture& getBitmapTex() const { return curr_cache->bitmap_texture; };
	void loadBitmapTex(unsigned px_size) const;

	const GlyphMetrics& getMetrics(unsigned char ch) const { return curr_cache->glyph_metrics[ch]; };
	Vec2<uint64_t>		getGlyphSize() const { return curr_cache->glyph_max_size; };
	unsigned			getPixelSize() const { return curr_cache->px_size; };

	int getYMin()	const { return curr_cache->yMin; };
	int getYMax()	const { return curr_cache->yMax; };
	int getHeight() const { return curr_cache->height; };

	bool isLoaded() const { return m_face != nullptr; }

	constexpr static uint8_t CHAR_COUNT = 128;

private:
	struct FontCache
	{
		FontCache(unsigned pixel_size) 
			: px_size{ pixel_size }
		{
		}
		~FontCache() {
			if (bitmap_surface) {
				SDL_DestroySurface(bitmap_surface);
			}
		}

		bool valid = false;

		int yMin = 0;
		int yMax = 0;
		int height = 0;
		unsigned px_size = 0;

		SDL_Surface* bitmap_surface = nullptr;
		Texture bitmap_texture;

		Vec2<uint64_t> glyph_max_size = { 0, 0 };
		std::array<GlyphMetrics, CHAR_COUNT> glyph_metrics;
	};

	FontCache* cache_for_size(unsigned size) const;

	FT_Face m_face = nullptr;

	mutable std::vector<std::unique_ptr<FontCache>> caches;
	mutable FontCache* curr_cache = nullptr;

};

}
