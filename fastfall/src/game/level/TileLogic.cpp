#include "fastfall/game/level/TileLogic.hpp"

#include "fastfall/util/log.hpp"

namespace ff {

namespace {

	using Map = std::map<std::string, TileLogicType, std::less<>>;
	using Iterator = std::map<std::string, TileLogicType>::iterator;

	std::unique_ptr<Map> factories;


	Map* getMap() {
		if (!factories) {
			factories = std::make_unique<Map>();
		}

		return factories.get();
	}

	std::unique_ptr<TileLogic> create(GameContext context, Iterator iter) {
		if (iter != getMap()->end()) {
			return iter->second.fn_create(context);
		}
		return nullptr;
	}
}

TileLogicType::TileLogicType(std::string_view type, FactoryFunction builder) :
	typeName(type), fn_create(builder)
{
	LOG_INFO("created type {}", type);
	addTileLogicType(*this);
}

void addTileLogicType(const TileLogicType& type) {
	getMap()->insert(std::make_pair(type.typeName, type));
}

std::unique_ptr<TileLogic> createTileLogic(GameContext context, std::string_view typeName) {
	return create(context, getMap()->find(typeName));
}

}
