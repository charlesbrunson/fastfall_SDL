#include "fastfall/game/WorldState.hpp"

#include "fastfall/game/WorldStateListener.hpp"

namespace ff {

void WorldState::adopt(ID<Entity> id, GenericID component)
{
	entities.at(id.value).components.push_back(component);
}


ID<Entity> WorldState::make_entity() {
	static const size_t ent_hash = typeid(Entity).hash_code();
	ID<Entity> id = { ent_hash, entities.emplace_back() };

	return id;
}

bool WorldState::erase_entity(ID<Entity> id)
{
	if (entities.exists(id.value))
	{
		auto& ent = entities.at(id.value);
		orphan_components.insert(
			orphan_components.end(),
			ent.components.begin(),
			ent.components.end()
		);
		entities.erase(id.value);
		return true;
	}
	return false;
}

void WorldState::push_listener(WorldStateListener* sub)
{
	if (std::find(listeners.begin(), listeners.end(), sub)
		== listeners.end())
	{
		listeners.push_back(sub);
	}
}

void WorldState::remove_listener(WorldStateListener* sub)
{
	std::erase(listeners, sub);
}

}

