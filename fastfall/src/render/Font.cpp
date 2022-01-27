#include "fastfall/render/Font.hpp"
#include "fastfall/render/freetype.hpp"
#include "fastfall/util/Rect.hpp"
#include "fastfall/util/log.hpp"

#include <vector>
#include <algorithm>
#include <bitset>

long get22_6p(long value)
{
	return (long)((double)value / 64.0);
}

long get16_16p(long value)
{
	return (long)((double)value / 65536.0);
}


void draw_glyph(FT_Bitmap bm, SDL_Surface* surf, int x, int y)
{
	if (bm.width == 0 || bm.rows == 0)
	{
		return;
	}

	std::vector<uint32_t> rgba((size_t)bm.rows * bm.width);

	for (size_t y = 0; y < bm.rows; ++y)
	{
		for (size_t x = 0; x < bm.width; ++x)
		{
			unsigned int c = (x / 8);

			std::bitset<8> val = bm.buffer[c + (y * bm.pitch)];

			if ( val.test(7 - (x % 8)) ) {
				rgba[(y * (size_t)bm.width) + x] = UINT32_MAX;
			}
		}
	}

	SDL_Surface* glyph = SDL_CreateRGBSurfaceFrom
	(
		&rgba[0],
		bm.width,
		bm.rows,
		32,
		bm.width * 4,
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
	);

	SDL_SetSurfaceBlendMode(glyph, SDL_BlendMode::SDL_BLENDMODE_BLEND);
	SDL_Rect dest = { x, y, 0, 0 };
	SDL_BlitSurface(glyph, nullptr, surf, &dest);
	SDL_FreeSurface(glyph);
}

namespace ff {

	Font::Font()
	{
	}

	Font::~Font()
	{
		if (m_face)
			FT_Done_Face(m_face);
	}

	bool Font::loadFromFile(std::string_view font_file)
	{
		unload();

		if (FT_New_Face(freetype_get_library(), font_file.data(), 0, &m_face))
		{
			LOG_ERR_("Failed to load face: {}", font_file);
			return false;
		}
		return true;
	}

	bool Font::loadFromStream(const void* font_data, short length)
	{
		unload();

		if (FT_New_Memory_Face(freetype_get_library(), (const FT_Byte*)font_data, length, 0, &m_face))
		{
			LOG_ERR_("Failed to load face from memory");
			return false;
		}
		return true;
	}


	void Font::unload()
	{
		if (m_face) {
			FT_Done_Face(m_face);
			m_face = nullptr;
		}

		caches.clear();
		curr_cache = nullptr;
	}


	bool Font::cachePixelSize(unsigned pixel_size)
	{
		auto prev = curr_cache;
		bool result = setPixelSize(pixel_size);
		curr_cache = prev;
		return result;
	}

	bool Font::setPixelSize(unsigned pixel_size)
	{
		if (pixel_size == 0)
		{
			return false;
		}

		auto it = std::find_if(caches.begin(), caches.end(), [&](auto& cache) {
			return cache->px_size == pixel_size;
		});

		if (it != caches.end())
		{
			curr_cache = it->get();
			return true;
		}
		else
		{
			curr_cache = cache_for_size(pixel_size);
			return true;
		}
		return false;
	}


	Font::FontCache* Font::cache_for_size(unsigned size)
	{
		if (!m_face)
		{
			LOG_ERR_("No font face loaded");
			return nullptr;
		}

		auto& cache = *caches.insert(
			std::lower_bound(caches.begin(), caches.end(), size,
				[](auto& cache, unsigned size) {
					return size > cache->px_size;
				}),
			std::make_unique<FontCache>(FontCache{.px_size = size})
		);

		if (FT_Set_Pixel_Sizes(m_face, 0, size))
		{
			LOG_ERR_("Failed to set pixel size");
			return nullptr;
		}

		for (unsigned i = 0; i < CHAR_COUNT; i++)
		{
			if (!FT_Load_Char(m_face, i, FT_LOAD_COMPUTE_METRICS))
			{
				auto& metrics = m_face->glyph->metrics;

				cache->glyph_max_size = {
					std::max(get22_6p(metrics.width),  (long)cache->glyph_max_size.x),
					std::max(get22_6p(metrics.height), (long)cache->glyph_max_size.y)
				};

				cache->glyph_metrics[i] = GlyphMetrics{
					.glyph_index = m_face->glyph->glyph_index,
					.size      = { get22_6p(metrics.width),			get22_6p(metrics.height) },
					.bearing   = { get22_6p(metrics.horiBearingX),	get22_6p(metrics.horiBearingY) },
					.advance_x = (unsigned)get22_6p(metrics.horiAdvance)
				};
			}
			else {
				LOG_ERR_("Failed to load face: {}", (char)i);
			}
		}

		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, cache->glyph_max_size.x * 16, cache->glyph_max_size.y * 8, 32, SDL_PIXELFORMAT_RGBA32);
		if (!surface) {
			LOG_ERR_("Failed to create SDL surface for font size");
			return cache.get();
		}

		cache->yMin	 = get22_6p(m_face->bbox.yMin);
		cache->yMax	 = get22_6p(m_face->bbox.yMax);
		cache->height = get22_6p(FT_MulFix(m_face->units_per_EM, m_face->size->metrics.y_scale));

		for (unsigned i = 0; i < CHAR_COUNT; i++)
		{
			if (!FT_Load_Char(m_face, i, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO))
			{
				// render glyph
				draw_glyph(
					m_face->glyph->bitmap,
					surface, 
					cache->glyph_max_size.x * (i % 16),
					cache->glyph_max_size.y * (i / 16)
				);
			}
		}

		//IMG_SavePNG(surface, "test.png");
		if (!cache->font_bitmap.loadFromSurface(surface))
		{
			LOG_ERR_("Failed to load texture from surface for font size");
		}
		SDL_FreeSurface(surface);

		cache->valid = true;
		return cache.get();
	}
}