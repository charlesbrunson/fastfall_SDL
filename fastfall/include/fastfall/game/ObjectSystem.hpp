#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/engine/time/time.hpp"

#include <list>
#include <map>

#include "fastfall/render/RenderTarget.hpp"

namespace ff {

class ObjectSystem {
public:
	ObjectSystem(unsigned instance);

	ObjectSystem(const ObjectSystem& obj);
	ObjectSystem(ObjectSystem&& obj) noexcept;

	ObjectSystem& operator=(const ObjectSystem& obj);
	ObjectSystem& operator=(ObjectSystem&& obj) noexcept;

	void update(secs deltaTime);
	void predraw(float interp, bool updated);

	void clear();

	inline ObjSpawnID getNextSpawnId() {
		return ObjSpawnID{ spawnCounter++ };
	}

	void addObject(std::unique_ptr<GameObject>&& obj);

	inline std::vector<std::unique_ptr<GameObject>>& getObjects() { return objects; };
	inline const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return objects; };

	inline unsigned getInstanceID() { return instanceID; };

private:
	unsigned instanceID;

	std::vector<std::unique_ptr<GameObject>> objects;
	std::vector<std::unique_ptr<GameObject>> created_objects;
	
	unsigned spawnCounter = 1;
};

}
