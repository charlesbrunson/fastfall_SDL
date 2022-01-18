#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string_view>
#include <optional>

namespace ff
{
	FT_Library freetype_get_library();

	void freetype_init();
	void freetype_quit();

	/*
	class FreeTypeFace
	{
	public:
		FreeTypeFace(std::string_view file_path, std::optional<unsigned> init_size = {});
		FreeTypeFace(const void* font_data, short length, std::optional<unsigned> init_size = {});
		~FreeTypeFace();

		bool set_pixel_size(unsigned size);
		unsigned get_pixel_size() { return px_size; };

		bool load_glyph(char ch);
		FT_GlyphSlot get_glyph();

		FT_Face face = nullptr;

	private:
		unsigned px_size = 0;
		std::optional<char> last_glyph;
	};
	*/

}