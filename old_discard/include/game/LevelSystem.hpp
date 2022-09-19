#pragma once

#include "fastfall/resource/asset/LevelAsset.hpp"
#include "level/Level.hpp"

#include <map>
#include <string>

namespace ff
{

class LevelSystem {
public:
	LevelSystem() = default;

	inline std::map<const std::string*, std::unique_ptr<Level>>& getAllLevels() noexcept { return currentLevels; };

	const std::string* addLevel(GameContext context, const LevelAsset& levelRef);

	bool eraseLevel(const std::string* level);

	Level* getActiveLevel();
	const Level* getActiveLevel() const;

	void clear();

	bool reloadLevels();

private:
	const std::string* activeLevel = nullptr;
	std::map<const std::string*, std::unique_ptr<Level>> currentLevels;
};

}
