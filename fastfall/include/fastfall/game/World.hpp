#pragma once


#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/game/InstanceID.hpp"

#include "fastfall/game/Level.hpp"
#include "fastfall/game/GameObjectManager.hpp"
#include "fastfall/game/CollisionManager.hpp"
#include "fastfall/game/TriggerManager.hpp"
#include "fastfall/game/GameCamera.hpp"
#include "fastfall/game/SceneManager.hpp"

#include "fastfall/render/RenderTarget.hpp"

#include <mutex>

namespace ff {

class World : public Drawable {
public:

	//static constexpr unsigned int NO_INSTANCE = 0u;

	World(InstanceID instance);

	// currently unsupported lmao
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	~World();

	void clear();
	void reset();

	inline Level* getActiveLevel() { return currentLevels.at(activeLevel).get(); };
	inline std::map<const std::string*, std::unique_ptr<Level>>& getAllLevels() noexcept { return currentLevels; };

	inline GameObjectManager& getObject() noexcept { return objMan; };
	inline CollisionManager& getCollision() noexcept { return colMan; };
	inline TriggerManager& getTrigger() noexcept { return triMan; };
	inline GameCamera& getCamera() noexcept { return camera; };
	inline SceneManager& getScene() noexcept { return sceneMan; };

	bool addLevel(const LevelAsset& levelRef);

	inline InstanceID getInstanceID() const noexcept { return instanceID; };

	inline GameContext getContext() const noexcept { return GameContext{ instanceID }; };

	void populateSceneFromLevel(Level& lvl);
	
	void update(secs deltaTime);
	void predraw(float interp, bool updated);


	bool want_reset = false;

private:
	void draw(RenderTarget& target, RenderState state = RenderState()) const override;

	GameCamera camera;

	const std::string* activeLevel = nullptr;
	std::map<const std::string*, std::unique_ptr<Level>> currentLevels;

	GameObjectManager objMan;
	CollisionManager colMan;
	TriggerManager triMan;
	SceneManager sceneMan;

	size_t update_counter = 0;

	World* ActiveWorld = nullptr;

	InstanceID instanceID;

};

World* Instance(InstanceID id);
World* CreateInstance();
void DestroyInstance(InstanceID id);
std::map<InstanceID, World>& AllInstances();

}