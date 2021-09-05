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
	activeLevel = nullptr;
	currentLevels.clear();
	objMan.clear();
	sceneMan.clear();
	camera.removeAllTargets();
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

/*
bool GameInstance::enableScissor(const RenderTarget& target, Vec2f viewPos) {
	glEnable(GL_SCISSOR_TEST);

	glm::fvec4 scissor;
	View view = target.getView();
	scissor = view.getViewport();

	Vec2f vpOff{ view.getViewport()[0], view.getViewport()[1] };
	Vec2f vpSize{ view.getViewport()[2], view.getViewport()[3] };

	if (getActiveLevel()) {
		float zoom = view.getViewport()[2] / view.getSize().x;

		glm::fvec2 levelsize = Vec2f{ getActiveLevel()->size() } *TILESIZE_F;
		glm::fvec2 campos = viewPos;
		glm::fvec2 campos2window = vpOff + (vpSize / 2.f);
		glm::fvec2 lvlbotleft2cam{ campos.x, levelsize.y - campos.y };
		glm::fvec2 levelbotleft2window = campos2window - (lvlbotleft2cam * zoom);

		scissor[0] = roundf(std::max(scissor[0], levelbotleft2window.x));
		scissor[1] = roundf(std::max(scissor[1], levelbotleft2window.y));

		scissor[2] = std::min(vpOff.x + vpSize.x, levelbotleft2window.x + (levelsize.x * zoom)) - scissor[0];
		scissor[3] = std::min(vpOff.y + vpSize.y, levelbotleft2window.y + (levelsize.y * zoom)) - scissor[1];

		scissor[2] = roundf(std::max(scissor[2], 0.f));
		scissor[3] = roundf(std::max(scissor[3], 0.f));
	}
	glScissor(
		scissor[0], 
		scissor[1], 
		scissor[2],
		scissor[3]
	);
	return scissor[2] > 0.f && scissor[3] > 0.f;
}

void GameInstance::disableScissor() {
	glDisable(GL_SCISSOR_TEST);
}
*/

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
