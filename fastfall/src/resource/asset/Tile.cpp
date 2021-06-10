#include "fastfall/resource/asset/Tile.hpp"

#include "fastfall/util/cardinal.hpp"
#include "fastfall/util/log.hpp"

#include <map>
#include <array>

namespace ff {

using StringTileType = std::pair<std::string_view, TileShape::Type>;
constexpr std::array<StringTileType, TILE_TYPE_COUNT> tileStringToType = {
	StringTileType{"empty",       TileShape::Type::EMPTY},
	StringTileType{"solid",       TileShape::Type::SOLID},
	StringTileType{"half",        TileShape::Type::HALF},
	StringTileType{"halfvert",    TileShape::Type::HALFVERT},
	StringTileType{"slope",       TileShape::Type::SLOPE},
	StringTileType{"shallow1",    TileShape::Type::SHALLOW1},
	StringTileType{"shallow2",    TileShape::Type::SHALLOW2},
	StringTileType{"steep1",      TileShape::Type::STEEP1},
	StringTileType{"steep2",      TileShape::Type::STEEP2},
	StringTileType{"oneway",      TileShape::Type::ONEWAY},
	StringTileType{"oneway_wall", TileShape::Type::ONEWAY_WALL},

	StringTileType{"levelboundary",      TileShape::Type::LEVELBOUNDARY},
	StringTileType{"levelboundary_wall",      TileShape::Type::LEVELBOUNDARY_WALL}
};

constexpr std::array<std::string_view, TILE_TYPE_COUNT> tileTypeToStr{
	"empty",
	"solid",
	"half",
	"slope",
	"shallow1",
	"shallow2",
	"steep1",
	"steep2",
	"oneway",
	"oneway_wall",

	"levelboundary",
	"levelboundary_wall"
};

TileShape::TileShape() noexcept :
	type(Type::EMPTY),
	shapeTouches(0u)
{

}

TileShape::TileShape(const char* shapeStr) noexcept {
	if (shapeStr == nullptr) return;

	type = Type::EMPTY;
	shapeTouches = 0u;

	std::string_view shapeString = shapeStr;
	int split = shapeString.find('-');

	std::string_view shapePrototype = shapeString.substr(0, split);
	std::string_view flipParams;
	if (split != std::string::npos) {
		flipParams = shapeString.substr(split + 1);
	}

	auto tileProto = std::find_if(tileStringToType.begin(), tileStringToType.end(),
		[&shapePrototype](const StringTileType& element) {
			return shapePrototype == element.first;
		});

	if (tileProto != tileStringToType.end()) {
		type = tileProto->second;
	}
	else {
		LOG_ERR_("could not find tile prototype: {}", shapeString);
		return;
	}

	hflipped = flipParams.find_first_of("hH") != std::string::npos;
	vflipped = flipParams.find_first_of("vV") != std::string::npos;

	init();
}

}