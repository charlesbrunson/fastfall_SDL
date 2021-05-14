#include "fastfall/resource/asset/Tile.hpp"

#include "fastfall/util/cardinal.hpp"
#include "fastfall/util/log.hpp"

#include <map>
#include <array>

/*
struct TilePrototype {
	TileShape::Type type;
	unsigned touchBits;
};
*/

namespace ff {

using StringTileType = std::pair<std::string_view, TileShape::Type>;
constexpr std::array<StringTileType, TILE_TYPE_COUNT> tileStringToType = {
	StringTileType{"EMPTY",       TileShape::Type::EMPTY},
	StringTileType{"SOLID",       TileShape::Type::SOLID},
	StringTileType{"HALF",        TileShape::Type::HALF},
	StringTileType{"HALFVERT",    TileShape::Type::HALFVERT},
	StringTileType{"SLOPE",       TileShape::Type::SLOPE},
	StringTileType{"SHALLOW1",    TileShape::Type::SHALLOW1},
	StringTileType{"SHALLOW2",    TileShape::Type::SHALLOW2},
	StringTileType{"STEEP1",      TileShape::Type::STEEP1},
	StringTileType{"STEEP2",      TileShape::Type::STEEP2},
	StringTileType{"ONEWAY",      TileShape::Type::ONEWAY},
	StringTileType{"ONEWAY_WALL", TileShape::Type::ONEWAY_WALL},

	StringTileType{"LEVELBOUNDARY",      TileShape::Type::LEVELBOUNDARY},
	StringTileType{"LEVELBOUNDARY_WALL",      TileShape::Type::LEVELBOUNDARY_WALL}
};

constexpr std::array<std::string_view, TILE_TYPE_COUNT> tileTypeToStr{
	"EMPTY",
	"SOLID",
	"HALF",
	"SLOPE",
	"SHALLOW1",
	"SHALLOW2",
	"STEEP1",
	"STEEP2",
	"ONEWAY",
	"ONEWAY_WALL",

	"LEVELBOUNDARY",
	"LEVELBOUNDARY_WALL"
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

	hflipped = flipParams.find('H') != std::string::npos;
	vflipped = flipParams.find('V') != std::string::npos;

	init();
}

}