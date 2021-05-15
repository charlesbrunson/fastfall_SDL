#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/engine/time/time.hpp"

#include <list>
#include <map>

#include "fastfall/render/RenderTarget.hpp"

namespace ff {

class GameObjectManager : public Drawable {
public:
	class ObjectDrawList : public Drawable {
	public:
		std::multimap<int, Drawable*> drawOrder;

	private:
		void draw(RenderTarget& target, RenderState states = RenderState()) const override {
			for (auto& drawable : drawOrder) {
				target.draw(*drawable.second, states);
			}
		}
	};


	GameObjectManager(unsigned instance);

	GameObjectManager(const GameObjectManager& obj);
	GameObjectManager(GameObjectManager&& obj) noexcept;

	GameObjectManager& operator=(const GameObjectManager& obj);
	GameObjectManager& operator=(GameObjectManager&& obj) noexcept;

	void update(secs deltaTime);
	void predraw(secs deltaTime);

	void clear();

	void addObject(std::unique_ptr<GameObject>&& obj);

	inline std::list<std::unique_ptr<GameObject>>& getObjects() { return objects; };
	inline unsigned getInstanceID() { return instanceID; };

	inline const ObjectDrawList& getObjectDrawList(bool foreground = false) const noexcept {
		return (foreground ? drawOrderFG : drawOrderBG);
	};

private:
	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

	unsigned instanceID;

	std::list<std::unique_ptr<GameObject>> objects;

	ObjectDrawList drawOrderFG; // objects ordered in front of the first FG tile layer
	ObjectDrawList drawOrderBG; // objects ordered in front of the first BG tile layer
};

}
