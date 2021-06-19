#pragma once

#include "fastfall/game/GameContext.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include <queue>
#include <string_view>

#include "fastfall/resource/asset/Tile.hpp"

namespace ff {


struct TileLogicCommand {
	enum class Type {
		Set,
		Remove
	};
	Type type;
	Vec2u position;

	Vec2u texposition;
	std::reference_wrapper<const TilesetAsset> tileset;
};

class TileLogic {
protected:

	inline void pushCommand(const TileLogicCommand& cmd) {
		commands.push(cmd);
	}

public:

	TileLogic(GameContext context, std::string name) 
		: m_context(context), m_name(name)
	{

	}
	virtual ~TileLogic() = default;

	virtual void addTile(Vec2u tilePos, Tile tile, std::string args) = 0;

	virtual void update(secs deltaTime) = 0;

	inline std::string_view getName() const {
		return m_name;
	}

	inline bool hasNextCommand() const {
		return !commands.empty();
	}
	inline const TileLogicCommand& nextCommand() const {
		return commands.front();
	}
	inline void popCommand() {
		commands.pop();
	}
	inline void clearCommands() {
		commands = std::queue<TileLogicCommand>{};
	}

private:
	std::string m_name;
	std::queue<TileLogicCommand> commands;
	GameContext m_context;
};

class TileLogicType {
public:
	using FactoryFunction = std::function<std::unique_ptr<TileLogic>(GameContext)>;

	TileLogicType(std::string_view type, FactoryFunction builder);

	std::string typeName;
	FactoryFunction fn_create;
};


void addTileLogicType(const TileLogicType& type);
std::unique_ptr<TileLogic> createTileLogic(GameContext context, std::string_view typeName);

}