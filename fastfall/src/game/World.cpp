#include "fastfall/game/World.hpp"
#include "fastfall/resource/Resources.hpp"

#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/ShapeRectangle.hpp"

namespace ff {

World::World(InstanceID instance) 
	: instanceID(instance)
	, activeLevel(nullptr)
	, objects(instance)
	, collisions(instance)
	, triggers(instance)
	, scene(instance)
{

}


World::~World() {
	clear();
	debug_draw::clear();
}

void World::clear() {
	objects.clear();
	scene.clear();
	camera.removeAllTargets();
	activeLevel = nullptr;
	currentLevels.clear();

	debug_draw::clear();

	update_counter = 0;
	collisions.resetFrameCount();
}

void World::reset()
{
	objects.clear();
	scene.clear();
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

	update_counter = 0;
	collisions.resetFrameCount();
}


bool World::addLevel(const LevelAsset& levelRef) {

	GameContext context{ getInstanceID() };
	auto r = currentLevels.emplace(std::make_pair(&levelRef.getAssetName(), std::make_unique<Level>(context, levelRef)));

	if (r.second && !activeLevel) {
		activeLevel = r.first->first;
		populateSceneFromLevel(*r.first->second);
		r.first->second->get_layers().get_obj_layer().createObjectsFromData(context);
	}
	return r.second;
}

void World::populateSceneFromLevel(Level& lvl)
{
	scene.clearType(SceneType::Level);
	auto& tile_layers = lvl.get_layers().get_tile_layers();
	for (auto& layer : tile_layers)
	{
		scene.add(SceneType::Level, layer.tilelayer, layer.position);
	}
	scene.set_bg_color(lvl.getBGColor());
	scene.set_size(lvl.size());
}

void World::update(secs deltaTime)
{
	if (activeLevel)
	{
		getActiveLevel()->update(deltaTime);
		triggers.update(deltaTime);
		objects.update(deltaTime);
		collisions.update(deltaTime);
		camera.update(deltaTime);
		update_counter++;
	}
}

void World::predraw(float interp, bool updated)
{
	if (want_reset)
		reset();

	if (activeLevel)
	{
		objects.predraw(interp, updated);
		getActiveLevel()->predraw(interp, updated);
		scene.set_cam_pos(getCamera().getPosition(interp));
	}
}

void World::draw(RenderTarget& target, RenderState state) const
{
	target.draw(scene, state);
}

///////////////////////////////////////////////////////////////////////////////////////////////

std::map<InstanceID, World> instances;
unsigned int instanceCounter = 1u;


World* Instance(InstanceID id) {
	auto r = instances.find(id);
	if (r != instances.end()) {
		return &r->second;
	}
	return nullptr;
}

World* CreateInstance() {
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

std::map<InstanceID, World>& AllInstances() {
	return instances;
}

}
