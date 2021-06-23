#pragma once

#include "fastfall/game/GameContext.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include <queue>
#include <string_view>

#include "fastfall/game/level/Tile.hpp"

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
	bool updateLogic = true;
};

class TileLogic;

class TileLogicType {
public:
	using FactoryFunction = std::function<std::unique_ptr<TileLogic>(GameContext)>;

	TileLogicType(std::string_view type, FactoryFunction builder);

	std::string typeName;
	FactoryFunction fn_create;
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
	virtual void removeTile(Vec2u tilePos) = 0;

	virtual void update(secs deltaTime) = 0;

	virtual bool on_precontact(Vec2i tilePos, const Contact& contact, secs duration) const {
		return true;
	};

	virtual void on_postcontact(Vec2i tilePos, const PersistantContact& contact) const {};


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



	template<typename T, typename = std::enable_if<std::is_base_of<TileLogic, T>::value>>
	static void addType(const std::string& typeName) {
		TileLogicType type{
			typeName,
			[](GameContext context) -> std::unique_ptr<TileLogic> {
				return std::make_unique<T>(context);
			}
		};
		getMap()->insert(std::make_pair(type.typeName, type));
	}
	static std::unique_ptr<TileLogic> create(GameContext context, std::string_view typeName);

private:

	using Map = std::map<std::string, TileLogicType, std::less<>>;
	using Iterator = std::map<std::string, TileLogicType>::iterator;

	static std::unique_ptr<Map> factories;

	static Map* getMap() {
		if (!factories) {
			factories = std::make_unique<Map>();
		}
		return factories.get();
	}

	std::string m_name;
	std::queue<TileLogicCommand> commands;
	GameContext m_context;
};



}