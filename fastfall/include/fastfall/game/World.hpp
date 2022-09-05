#pragma once



#include "fastfall/game/InstanceID.hpp"

#include "fastfall/game/WorldState.hpp"
#include "fastfall/game/ObjectSystem.hpp"
#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/game/LevelSystem.hpp"

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

	LevelSystem		levels;
	ObjectSystem	objects;
	CollisionSystem collisions;
	TriggerSystem	triggers;
	CameraSystem	camera;
	SceneSystem		scene;

	inline InstanceID getInstanceID() const noexcept { return instanceID; };
	inline GameContext getContext() const noexcept { return GameContext{ instanceID }; };
		
	void update(secs deltaTime);
	void predraw(float interp, bool updated);

	bool want_reset = false;

private:
	void draw(RenderTarget& target, RenderState state = RenderState()) const override;

	size_t update_counter = 0;

	InstanceID instanceID;
};

World* Instance(InstanceID id);
World* CreateInstance();
void DestroyInstance(InstanceID id);
std::map<InstanceID, World>& AllInstances();

}