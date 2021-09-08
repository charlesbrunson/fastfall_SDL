#include "fastfall/game/Instance.hpp"
#include "fastfall/resource/Resources.hpp"

#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/ShapeRectangle.hpp"

namespace ff {

GameInstance::GameInstance(InstanceID instance) :
	instanceID(instance),
	activeLevel(nullptr),
	objMan(instance),
	colMan(instance),
	triMan(instance),
	sceneMan(instance)
{

}


GameInstance::~GameInstance() {
	clear();
}

void GameInstance::clear() {
	objMan.clear();
	sceneMan.clear();
	camera.removeAllTargets();
	activeLevel = nullptr;
	currentLevels.clear();
}

void GameInstance::reset() {

	//activeLevel = nullptr;
	objMan.clear();
	sceneMan.clear();
	camera.removeAllTargets();
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
		populateSceneFromLevel(*lvl);
	}

}


bool GameInstance::addLevel(const LevelAsset& levelRef) {
	auto r = currentLevels.emplace(std::make_pair(&levelRef.getAssetName(), std::make_unique<Level>(GameContext{ getInstanceID() }, levelRef)));

	if (r.second && !activeLevel) {
		activeLevel = r.first->first;
		populateSceneFromLevel(*r.first->second);
	}
	return r.second;
}

void GameInstance::populateSceneFromLevel(Level& lvl) {
	int bg_count = lvl.getBGLayers().size();
	//int fg_count = lvl.getFGLayers().size();

	sceneMan.clearType(SceneType::Level);

	for (int bg_ndx = 0; auto& layer : lvl.getBGLayers()) {
		//LOG_INFO("{}", 1 + bg_ndx);
		sceneMan.add(SceneType::Level, layer, bg_count - bg_ndx);
		bg_ndx++;
	}

	
	for (int fg_ndx = 0; auto& layer : lvl.getFGLayers()) {
		//LOG_INFO("{}", -fg_ndx);
		sceneMan.add(SceneType::Level, layer, -fg_ndx);
		fg_ndx++;
	}
	sceneMan.set_bg_color(lvl.getBGColor());
	sceneMan.set_size(lvl.size());

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

	auto [iter, emplaced] = instances.emplace(id, id);

	instanceCounter++;
	if (emplaced)
		return &iter->second;
	else
		return nullptr;
}

void DestroyInstance(InstanceID id) {
	if (auto* inst = Instance(id))
	{
		// clear first for a cleaner exit
		inst->clear();	
	}
	instances.erase(id);
}

std::map<InstanceID, GameInstance>& AllInstances() {
	return instances;
}

}
