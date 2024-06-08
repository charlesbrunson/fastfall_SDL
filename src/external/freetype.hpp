#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

#include <string_view>
#include <optional>

namespace ff
{
	FT_Library freetype_get_library();

	void freetype_init();
	void freetype_quit();

}