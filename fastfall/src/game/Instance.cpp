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
	debug_draw::clear();
}

void GameInstance::clear() {
	objMan.clear();
	sceneMan.clear();
	camera.removeAllTargets();
	activeLevel = nullptr;
	currentLevels.clear();

	debug_draw::clear();

	update_counter = 0;
}

void GameInstance::reset() 
{

	update_counter = 0;

	objMan.clear();
	sceneMan.clear();
	debug_draw::clear();
	camera.removeAllTargets();
	for (auto& lvl : currentLevels) {
		if (!lvl.second->name().empty()) {
			auto* asset = Resources::get<LevelAsset>(lvl.second->name());
			if (asset) {
				lvl.second->init(*asset);
			}
		}
	}

	Level* lvl = getActiveLevel();
	assert(lvl);

	if (lvl) {
		populateSceneFromLevel(*lvl);
		lvl->get_layers().get_obj_layer().createObjectsFromData(GameContext{ instanceID });
	}
	want_reset = false;
}


bool GameInstance::addLevel(const LevelAsset& levelRef) {

	GameContext context{ getInstanceID() };
	auto r = currentLevels.emplace(std::make_pair(&levelRef.getAssetName(), std::make_unique<Level>(context, levelRef)));

	if (r.second && !activeLevel) {
		activeLevel = r.first->first;
		populateSceneFromLevel(*r.first->second);
		r.first->second->get_layers().get_obj_layer().createObjectsFromData(context);
	}
	return r.second;
}

void GameInstance::populateSceneFromLevel(Level& lvl)
{
	sceneMan.clearType(SceneType::Level);
	auto& tile_layers = lvl.get_layers().get_tile_layers();
	for (auto& layer : tile_layers)
	{
		sceneMan.add(SceneType::Level, layer.tilelayer, layer.position);
	}
	sceneMan.set_bg_color(lvl.getBGColor());
	sceneMan.set_size(lvl.size());
}

void GameInstance::update(secs deltaTime)
{
	if (activeLevel)
	{
		getActiveLevel()->update(deltaTime);
		objMan.update(deltaTime);
		triMan.update(deltaTime);
		colMan.update(deltaTime);
		camera.update(deltaTime);
		//LOG_INFO("{}", update_counter);
		update_counter++;
	}
}

void GameInstance::predraw(float interp, bool updated)
{
	if (want_reset)
		reset();

	if (activeLevel)
	{
		objMan.predraw(interp, updated);
		getActiveLevel()->predraw(interp, updated);
		sceneMan.set_cam_pos(getCamera().getPosition(interp));
	}
}

void GameInstance::draw(RenderTarget& target, RenderState state) const
{
	target.draw(sceneMan, state);
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
