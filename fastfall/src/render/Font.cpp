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
		: glyph_metrics{}
		, glyph_max_size{0, 0}
	{
	}

	bool Font::loadFromFile(std::string_view font_file, unsigned pixel_size)
	{
		FT_Face face;
		if (FT_New_Face(freetype_get_library(), font_file.data(), 0, &face))
		{
			LOG_ERR_("Failed to load face: {}", font_file);
			return false;
		}

		if (FT_Set_Pixel_Sizes(face, 0, pixel_size))
		{
			LOG_ERR_("Failed to set pixel size");
			return false;
		}
		px_size = pixel_size;

		return load(face);
	}

	bool Font::loadFromStream(const void* font_data, short length, unsigned pixel_size)
	{
		FT_Face face;
		if (FT_New_Memory_Face(freetype_get_library(), (const FT_Byte*)font_data, length, 0, &face))
		{
			LOG_ERR_("Failed to load face from memory");
			return false;
		}

		if (FT_Set_Pixel_Sizes(face, 0, pixel_size))
		{
			LOG_ERR_("Failed to set pixel size");
			return false;
		}
		px_size = pixel_size;

		return load(face);
	}

	bool Font::load(FT_Face face)
	{
		std::array<GlyphMetrics, CHAR_COUNT> calc_metrics{};
		glm::i64vec2 calc_size = { 0,0 };

		for (unsigned i = 0; i < CHAR_COUNT; i++)
		{
			if (!FT_Load_Char(face, i, FT_LOAD_COMPUTE_METRICS))
			{
				auto& metrics = face->glyph->metrics;

				calc_size = {
					std::max(get22_6p(metrics.width),  (long)calc_size.x),
					std::max(get22_6p(metrics.height), (long)calc_size.y)
				};

				calc_metrics[i] = GlyphMetrics{
					.size      = { get22_6p(metrics.width),			get22_6p(metrics.height) },
					.bearing   = { get22_6p(metrics.horiBearingX),	get22_6p(metrics.horiBearingY) },
					.advance_x = (unsigned)get22_6p(metrics.horiAdvance)
				};
			}
		}

		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, calc_size.x * 16, calc_size.y * 8, 32, SDL_PIXELFORMAT_RGBA32);
		if (!surface)
			return false;

		glyph_max_size = calc_size;
		glyph_metrics = calc_metrics;

		for (unsigned i = 0; i < CHAR_COUNT; i++)
		{
			if (!FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO))
			{
				// render glyph
				draw_glyph(
					face->glyph->bitmap,
					surface, 
					glyph_max_size.x * (i % 16),
					glyph_max_size.y * (i / 16)
				);
			}
		}

		IMG_SavePNG(surface, "test.png");
		font_bitmap.loadFromSurface(surface);
		SDL_FreeSurface(surface);
		return true;
	}

}