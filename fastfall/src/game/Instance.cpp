#include "fastfall/game/Instance.hpp"
#include "fastfall/resource/Resources.hpp"

namespace ff {

GameInstance::GameInstance(InstanceID instance) :
	instanceID(instance),
	activeLevel(nullptr),
	objMan(instance),
	colMan(instance)
{

}


GameInstance::~GameInstance() {
	clear();
}

void GameInstance::clear() {
	activeLevel = nullptr;
	currentLevels.clear();
	objMan.clear();
	camera.removeAllTargets();
}

void GameInstance::reset() {

	//activeLevel = nullptr;
	camera.removeAllTargets();
	objMan.clear();
	for (auto& lvl : currentLevels) {
		if (lvl.second->name()) {
			auto* asset = Resources::get<LevelAsset>(*lvl.second->name());
			if (asset) {
				lvl.second->init(*asset);
			}
		}
	}

	Level* lvl = getActiveLevel();
	assert(lvl);
	if (lvl) {

		if (!lvl->getFGLayers().empty())
			getCollision().addColliderRegion(lvl->getFGLayers().begin()->getCollisionMap());

	}

}


bool GameInstance::addLevel(const LevelAsset& levelRef) {
	auto r = currentLevels.emplace(std::make_pair(&levelRef.getAssetName(), std::make_unique<Level>(GameContext{ getInstanceID() }, levelRef)));

	if (r.second && !activeLevel) {
		activeLevel = r.first->first;
	}
	return r.second;
}

///////////////////////////////////////////////////////////////////////////////////////////////

std::map<InstanceID, GameInstance> instances;
unsigned int instanceCounter = 1u;


GameInstance* Instance(InstanceID id) {
	auto r = instances.find(id);
	if (r != instances.end()) {
		return &r->second;
	}
	return nullptr;
}

GameInstance* CreateInstance() {
	InstanceID id{ instanceCounter };
	auto [iter, emplaced] = instances.emplace(id, id); // instanceCounter, GameInstance(instanceCounter)

	instanceCounter++;
	if (emplaced)
		return &iter->second;
	else
		return nullptr;
}

void DestroyInstance(InstanceID id) {
	instances.erase(id);
}

std::map<InstanceID, GameInstance>& AllInstances() {
	return instances;
}

}