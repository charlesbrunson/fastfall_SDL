#include "fastfall/game/level/TileLogic.hpp"

#include "fastfall/util/log.hpp"

namespace ff {

TileLogicType::TileLogicType(std::string_view type, FactoryFunction builder) :
	typeName(type), fn_create(builder)
{

}

std::unique_ptr<TileLogic> TileLogic::create(WorldState& st, std::string_view typeName) {
	auto iter = getMap().find(typeName);
	if (iter != getMap().end()) {
		return iter->second.fn_create(st);
	}
	return nullptr;
}

}
