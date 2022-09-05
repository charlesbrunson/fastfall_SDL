#include "fastfall/game/World.hpp"
#include "fastfall/resource/Resources.hpp"

#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/ShapeRectangle.hpp"

namespace ff {

World::World(InstanceID instance) 
	: instanceID(instance)
{
}


World::~World() {
	clear();
	debug_draw::clear();
}

void World::clear() {
	objects.clear();
	levels.clear();

	debug_draw::clear();

	update_counter = 0;
	collisions.resetFrameCount();
}

void World::reset()
{
	objects.clear();
	debug_draw::clear();
	levels.reloadLevels();

	if (auto* active = levels.getActiveLevel()) {
		//populateSceneFromLevel(*active);
		active->get_layers().get_obj_layer().createObjectsFromData(GameContext{ instanceID });
	}
	want_reset = false;

	update_counter = 0;
	collisions.resetFrameCount();
}

/*
void World::populateSceneFromLevel(Level& lvl)
{
	
	//scene.clearType(SceneType::Level);
	auto& tile_layers = lvl.get_layers().get_tile_layers();

	for (auto& layer : tile_layers)
	{
		//scene.add(SceneType::Level, layer.tilelayer, layer.position);
	}
	

}
*/

void World::update(secs deltaTime)
{
	if (auto* active = levels.getActiveLevel())
	{
		active->update(deltaTime);
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

	if (auto* active = levels.getActiveLevel())
	{
		scene.set_bg_color(active->getBGColor());
		scene.set_size(active->size());
		objects.predraw(interp, updated);
		active->predraw(interp, updated);
		scene.set_cam_pos(camera.getPosition(interp));
	}
	else
	{
		scene.set_bg_color(ff::Color::Transparent);
		//scene.set_size(active->size());
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
