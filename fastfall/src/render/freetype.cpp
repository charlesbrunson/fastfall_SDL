#include "fastfall/render/freetype.hpp"
#include "fastfall/util/log.hpp"

namespace ff
{

	FT_Library library;

	FT_Library freetype_get_library()
	{
		return library;
	}

	void freetype_init()
	{
		if (FT_Init_FreeType(&library))
		{
			LOG_ERR_("Failed to initialize FreeType");
		}

		FT_Int ft_major, ft_minor, ft_patch;
		FT_Library_Version(library, &ft_major, &ft_minor, &ft_patch);
		LOG_INFO("{:>10} {}.{}.{}", "FreeType", ft_major, ft_minor, ft_patch);
	}

	void freetype_quit()
	{
		if (FT_Done_FreeType(library))
		{
			LOG_ERR_("Failed to destroy FreeType");
		}
	}

	/*
	FreeTypeFace::FreeTypeFace(std::string_view file_path, std::optional<unsigned> init_size)
	{
		if (FT_New_Face(library, file_path.data(), 0, &face))
		{
			LOG_ERR_("Failed to load face: {}", file_path);
		}
		if (init_size)
		{
			set_pixel_size(*init_size);
		}
	}

	FreeTypeFace::FreeTypeFace(const void* font_data, short length, std::optional<unsigned> init_size)
	{
		if (FT_New_Memory_Face(library, (const FT_Byte*)font_data, length, 0, &face))
		{
			LOG_ERR_("Failed to load face from memory");
		}
		if (init_size)
		{
			set_pixel_size(*init_size);
		}
	}

	FreeTypeFace::~FreeTypeFace()
	{
		FT_Done_Face(face);
	}

	bool FreeTypeFace::set_pixel_size(unsigned size)
	{
		if (FT_Set_Pixel_Sizes(face, 0, size))
		{
			LOG_ERR_("Failed to set pixel size");
			return false;
		}
		px_size = size;
		last_glyph = {};
		return true;
	}

	bool FreeTypeFace::load_glyph(char ch)
	{
		if (!last_glyph || last_glyph.value() != ch) {
			if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
			{
				LOG_ERR_("Failed to load char");
				return false;
			}
		}
		return true;
	}

	FT_GlyphSlot FreeTypeFace::get_glyph()
	{
		return face->glyph;
	}
	*/
}