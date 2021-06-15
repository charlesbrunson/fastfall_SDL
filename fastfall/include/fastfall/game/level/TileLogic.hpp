#pragma once

#include "fastfall/game/GameContext.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include <queue>
#include <string_view>

#include "fastfall/resource/asset/Tile.hpp"

namespace ff {



class TileLogic {
protected:
	struct Command {
		enum class Type {
			Set,
			Remove
		};
		Type type;
		Vec2u& position;

		Vec2u& texposition;
		TilesetAsset& tileset;
	};

	inline void pushCommand(const Command& cmd) {
		commands.push(cmd);
	}

public:

	TileLogic(GameContext context) 
		: m_context(context)
	{

	}

	virtual void addTile(Vec2u tilePos, std::string args) = 0;

	virtual void updateLogic(secs deltaTime) = 0;

	inline bool hasNextCommand() const {
		return !commands.empty();
	}
	inline const Command& nextCommand() const {
		return commands.front();
	}
	inline void popCommand() {
		commands.pop();
	}
	inline void clearCommands() {
		commands = std::queue<Command>{};
	}

private:
	std::queue<Command> commands;
	GameContext m_context;
};

class TileLogicType {
public:
	using FactoryFunction = std::function<std::unique_ptr<TileLogic>(GameContext)>;

	TileLogicType(std::string_view type, FactoryFunction create) {
		typeName = type;
		fn_create = create;
	}

	std::string_view typeName;
	FactoryFunction fn_create;
};

void addTileLogicType(const TileLogicType& type);

std::unique_ptr<TileLogic> createTileLogic(GameContext context, std::string_view typeName);

}