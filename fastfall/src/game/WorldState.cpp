#include "fastfall/game/WorldState.hpp"

namespace ff {

void WorldState::adopt(ID<Entity> id, GenericID component)
{
	entities.at(id.value).components.push_back(component);
}


ID<Entity> WorldState::make_entity() {
	static const size_t ent_hash = typeid(Entity).hash_code();
	ID<Entity> id = { ent_hash, entities.emplace_back() };

	for (auto& sub : subscribers)
	{
		sub->notify_entity_created(id);
	}
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

void WorldState::push_subscriber(WorldStateSubcriber* sub)
{
	if (std::find(subscribers.begin(), subscribers.end(), sub)
		== subscribers.end())
	{
		subscribers.push_back(sub);
	}
}

void WorldState::remove_subscriber(WorldStateSubcriber* sub)
{
	std::erase(subscribers, sub);
}

}

