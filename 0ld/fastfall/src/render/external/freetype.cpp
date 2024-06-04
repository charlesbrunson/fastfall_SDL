#include "fastfall/render/external/freetype.hpp"
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

}