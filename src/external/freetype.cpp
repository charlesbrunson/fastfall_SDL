#include "freetype.hpp"
#include "ff/util/log.hpp"

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
			ff::error("Failed to initialize FreeType");
		}

		FT_Int ft_major, ft_minor, ft_patch;
		FT_Library_Version(library, &ft_major, &ft_minor, &ft_patch);
        ff::info("{}: {}.{}.{}", "FreeType", ft_major, ft_minor, ft_patch);
	}

	void freetype_quit()
	{
		if (FT_Done_FreeType(library))
		{
            ff::error("Failed to destroy FreeType");
		}
	}

}