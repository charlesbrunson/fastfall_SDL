#include "fastfall/game/level/Tile.hpp"

//#include "fastfall/util/direction.hpp"
#include "fastfall/util/log.hpp"

//#include <map>
#include <unordered_map>
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
	StringTileType{"onewayvert",  TileShape::Type::ONEWAYVERT},

	StringTileType{"levelboundary",      TileShape::Type::LEVELBOUNDARY},
	StringTileType{"levelboundary_wall", TileShape::Type::LEVELBOUNDARY_WALL}
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

// tile materials
namespace {
	std::unordered_map<std::string, TileMaterial> materials;
}

const TileMaterial Tile::standardMat = {
	.typeName = "standard"
};

void Tile::addMaterial(const TileMaterial& mat) {
	materials.insert_or_assign(mat.typeName, mat);
}

const TileMaterial& Tile::getMaterial(std::string typeName) {
	const auto it = materials.find(typeName);

	if (it != materials.end()) {
		return (it->second);
	}
	else {
		return standardMat;
	}
}

}
