#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/engine/time/time.hpp"

#include <list>
#include <map>

#include "fastfall/render/RenderTarget.hpp"

namespace ff {

class GameObjectManager {
public:
	GameObjectManager(unsigned instance);

	GameObjectManager(const GameObjectManager& obj);
	GameObjectManager(GameObjectManager&& obj) noexcept;

	GameObjectManager& operator=(const GameObjectManager& obj);
	GameObjectManager& operator=(GameObjectManager&& obj) noexcept;

	void update(secs deltaTime);
	void predraw(secs deltaTime);

	void clear();

	void addObject(std::unique_ptr<GameObject>&& obj);

	inline std::vector<std::unique_ptr<GameObject>>& getObjects() { return objects; };
	inline const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return objects; };

	inline unsigned getInstanceID() { return instanceID; };

private:
	unsigned instanceID;

	std::vector<std::unique_ptr<GameObject>> objects;
	
};

}
