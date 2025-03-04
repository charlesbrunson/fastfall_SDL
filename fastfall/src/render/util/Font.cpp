#include "fastfall/render/util/Font.hpp"
#include "fastfall/render/external/freetype.hpp"
#include "fastfall/util/Rect.hpp"
#include "fastfall/util/log.hpp"

#include <vector>
#include <algorithm>
#include <bitset>
#include <string_view>

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


	SDL_Surface* glyph = SDL_CreateSurfaceFrom(
		bm.width,
		bm.rows,
		SDL_PIXELFORMAT_RGBA8888,
		rgba.data(),
		bm.width
	);

	SDL_SetSurfaceBlendMode(glyph, SDL_BLENDMODE_BLEND);
	SDL_Rect dest = { x, y, 0, 0 };
	SDL_BlitSurface(glyph, nullptr, surf, &dest);
	SDL_DestroySurface(glyph);
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

    bool Font::loadFromFile(const std::filesystem::path& font_path)
    {
        unload();
        std::string str{ font_path.generic_string() };
        if (FT_New_Face(freetype_get_library(), str.data(), 0, &m_face))
        {
            LOG_ERR_("Failed to load face: {}", str.data());
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


	bool Font::cachePixelSize(unsigned pixel_size) const
	{
		auto prev = curr_cache;
		bool result = setPixelSize(pixel_size);
		curr_cache = prev;
		return result;
	}

	bool Font::setPixelSize(unsigned pixel_size) const
	{
		if (pixel_size == 0)
		{
			return false;
		}

        //caches.begin();
        //caches.end();

		auto it = std::find_if(
            caches.begin(),
            caches.end(),
           [&](auto& cache) {
                return cache->px_size == pixel_size;
            }
        );

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


	Font::FontCache* Font::cache_for_size(unsigned size) const
	{
		if (!m_face)
		{
			LOG_ERR_("No font face loaded");
			return nullptr;
		}

		auto& cache = *caches.insert(
			std::lower_bound(caches.begin(), caches.end(), size,
				[](const auto& cache, unsigned size) { return size > cache->px_size; }),
			std::make_unique<FontCache>(size)
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

		cache->bitmap_surface = SDL_CreateSurface(
			cache->glyph_max_size.x * 16,
			cache->glyph_max_size.y * 8,
			SDL_PIXELFORMAT_RGBA32
		);
		// cache->bitmap_surface = SDL_CreateRGBSurfaceWithFormat(0, cache->glyph_max_size.x * 16, cache->glyph_max_size.y * 8, 32, SDL_PIXELFORMAT_RGBA32);

		if (!cache->bitmap_surface) {
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
					cache->bitmap_surface,
					cache->glyph_max_size.x * (i % 16),
					cache->glyph_max_size.y * (i / 16)
				);
			}
		}

		cache->valid = true;
		return cache.get();
	}

	void Font::loadBitmapTex(unsigned px_size) const
	{
		auto it = std::find_if(caches.begin(), caches.end(), [&](auto& cache) {
			return cache->px_size == px_size;
			});

		FontCache* cache;
		if (it != caches.end())
		{
			cache = it->get();

			if (cache->valid 
				&& cache->bitmap_surface 
				&& !cache->bitmap_texture.exists()) 
			{
				if (!cache->bitmap_texture.loadFromSurface(cache->bitmap_surface))
				{
					LOG_ERR_("Failed to load texture from surface for font size");
				}
				SDL_DestroySurface(cache->bitmap_surface);
				cache->bitmap_surface = nullptr;

			}
		}
	}
}
