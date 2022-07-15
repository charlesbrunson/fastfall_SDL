#pragma once


#include "fastfall/resource/asset/LevelAsset.hpp"

//#include "fastfall/game/InstanceID.hpp"

#include "fastfall/game/WorldState.hpp"
#include "fastfall/game/Level.hpp"
//#include "fastfall/game/ObjectSystem.hpp"
#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/game/CollidableSystem.hpp"

#include "fastfall/render/RenderTarget.hpp"

#include <mutex>

namespace ff {

class World : public Drawable {
public:
	void clear();
	void reset();

	WorldState& state() { return state_; };
	const WorldState& state() const { return state_; };

	bool addLevel(const LevelAsset& levelRef);

	void populateSceneFromLevel(Level& lvl);
	
	void update(secs deltaTime);
	void predraw(float interp, bool updated);

	bool want_reset = false;

private:
	void draw(RenderTarget& target, RenderState state = RenderState()) const override;

	const std::string* activeLevel = nullptr;
	size_t update_counter = 0;

	std::map<const std::string*, std::unique_ptr<Level>> currentLevels;

	CameraSystem camera;
	CollisionSystem collisions;
	CollidableSystem collidables;
	TriggerSystem triggers;
	SceneSystem scene;

	WorldState state_;
};	

}