#pragma once


#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/game/InstanceID.hpp"

#include "fastfall/game/Level.hpp"
#include "fastfall/game/GameObjectManager.hpp"
#include "fastfall/game/CollisionManager.hpp"
#include "fastfall/game/GameCamera.hpp"

#include "fastfall/render/RenderTarget.hpp"

namespace ff {

class GameInstance {
public:

	//static constexpr unsigned int NO_INSTANCE = 0u;

	GameInstance(InstanceID instance);
	~GameInstance();

	void clear();
	void reset();

	inline Level* getActiveLevel() { return currentLevels.at(activeLevel).get(); };
	inline std::map<const std::string*, std::unique_ptr<Level>>& getAllLevels() noexcept { return currentLevels; };

	inline GameObjectManager& getObject() noexcept { return objMan; };
	inline CollisionManager& getCollision() noexcept { return colMan; };
	inline GameCamera& getCamera() noexcept { return camera; };

	bool addLevel(const LevelAsset& levelRef);

	inline InstanceID getInstanceID() const noexcept { return instanceID; };

	inline GameContext getContext() const noexcept { return GameContext{ instanceID }; };


	bool enableScissor(const RenderTarget& target, Vec2f viewPos);
	void disableScissor();

private:
	InstanceID instanceID;

	//unsigned int activelevel;

	GameCamera camera;

	// pointer to level name (from level asset), Level
	const std::string* activeLevel = nullptr;
	std::map<const std::string*, std::unique_ptr<Level>> currentLevels;

	GameObjectManager objMan;
	CollisionManager colMan;

};


GameInstance* Instance(InstanceID id);
GameInstance* CreateInstance();
void DestroyInstance(InstanceID id);
std::map<InstanceID, GameInstance>& AllInstances();

}