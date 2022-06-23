#pragma once


#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/game/InstanceID.hpp"

#include "fastfall/game/Level.hpp"
#include "fastfall/game/ObjectSystem.hpp"
#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/SceneSystem.hpp"

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

	inline ObjectSystem& getObject()		noexcept { return objects;		};
	inline CollisionSystem& getCollision()	noexcept { return collisions;	};
	inline TriggerSystem& getTrigger()		noexcept { return triggers;		};
	inline CameraSystem& getCamera()		noexcept { return camera;		};
	inline SceneSystem& getScene()			noexcept { return scene;		};

	bool addLevel(const LevelAsset& levelRef);

	inline InstanceID getInstanceID() const noexcept { return instanceID; };

	inline GameContext getContext() const noexcept { return GameContext{ instanceID }; };

	void populateSceneFromLevel(Level& lvl);
	
	void update(secs deltaTime);
	void predraw(float interp, bool updated);


	bool want_reset = false;

private:
	void draw(RenderTarget& target, RenderState state = RenderState()) const override;

	const std::string* activeLevel = nullptr;
	size_t update_counter = 0;

	InstanceID instanceID;

	std::map<const std::string*, std::unique_ptr<Level>> currentLevels;

	ObjectSystem	objects;
	CollisionSystem collisions;
	TriggerSystem	triggers;
	CameraSystem	camera;
	SceneSystem		scene;

};

World* Instance(InstanceID id);
World* CreateInstance();
void DestroyInstance(InstanceID id);
std::map<InstanceID, World>& AllInstances();

}