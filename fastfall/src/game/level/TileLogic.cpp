#include "fastfall/game/level/TileLogic.hpp"

#include "fastfall/util/log.hpp"

namespace ff {

TileLogicType::TileLogicType(std::string_view type, FactoryFunction builder) :
	typeName(type), fn_create(builder)
{

}

copyable_unique_ptr<TileLogic>&& TileLogic::create(World& world, std::string_view typeName) {
	auto iter = getMap().find(typeName);
	if (iter != getMap().end()) {
        auto ptr = std::move(iter->second.fn_create(world));
		return std::move(ptr);
	}
    return copyable_unique_ptr<TileLogic>{};
}

}
