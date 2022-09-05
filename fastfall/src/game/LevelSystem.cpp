#include "fastfall/game/LevelSystem.hpp"

#include "fastfall/resource/Resources.hpp"

namespace ff 
{


const std::string* LevelSystem::addLevel(GameContext context, const LevelAsset& levelRef)
{
	auto [iter, inserted] = currentLevels.emplace(std::make_pair(&levelRef.getAssetName(), std::make_unique<Level>(context, levelRef)));

	if (inserted && !activeLevel) {
		activeLevel = iter->first;
		iter->second->get_layers().get_obj_layer().createObjectsFromData(context);
	}
	return inserted ? iter->first : nullptr;
};

bool LevelSystem::eraseLevel(const std::string* level)
{
	if (activeLevel == level)
	{
		activeLevel = nullptr;
	}
	return currentLevels.erase(level) > 0;
}

Level* LevelSystem::getActiveLevel()
{
	return activeLevel ? currentLevels.at(activeLevel).get() : nullptr;
}

const Level* LevelSystem::getActiveLevel() const
{
	return activeLevel ? currentLevels.at(activeLevel).get() : nullptr;
}

void LevelSystem::clear()
{
	currentLevels.clear();
	activeLevel = nullptr;
}

bool LevelSystem::reloadLevels()
{
	bool any_reloaded = false;
	for (auto& lvl : currentLevels) {
		if (!lvl.second->name().empty()) {
			auto* asset = Resources::get<LevelAsset>(lvl.second->name());
			if (asset) {
				lvl.second->init(*asset);
				any_reloaded = true;
			}
		}
	}
	return any_reloaded;
}


}
