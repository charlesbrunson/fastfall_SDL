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

	class FreeTypeFace
	{
	public:
		FreeTypeFace(std::string_view file_path, std::optional<unsigned> init_size = {});
		~FreeTypeFace();

		bool set_pixel_size(unsigned size);
		std::optional<FT_Bitmap> get_glyph_bitmap(char ch);

		FT_Face face = nullptr;

	private:
		std::optional<char> last_glyph;
	};

}