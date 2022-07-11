#pragma once

#include "fastfall/game/ComponentList.hpp"

#include "fastfall/game/camera/CameraTarget.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/render/Drawable.hpp"
#include "fastfall/game/trigger/Trigger.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"

#include <span>

namespace ff {

struct Entity {
	ID<Entity> m_id;
	std::vector<GenericID> components;
};

class WorldStateSubcriber
{
public:
	virtual void notify_component_created(GenericID id) {};
	virtual void notify_component_destroyed(GenericID id) {};

	virtual void notify_entity_created(ID<Entity> id) {};
	virtual void notify_entity_destroyed(ID<Entity> id) {};
};


class WorldState
{
private:

	ComponentList<CameraTarget, true>	camtargets;
	ComponentList<Collidable, false>	collidables;
	ComponentList<ColliderRegion, true>	colliders;
	ComponentList<Drawable, true>		drawables;
	ComponentList<Trigger, false>		triggers;

	std::vector<GenericID> orphan_components;
	slot_map<Entity> entities;

	std::vector<WorldStateSubcriber*> subscribers;

public:

	// entities
	ID<Entity> make_entity();
	bool erase_entity(ID<Entity> id);

	// components
	// camtarget
	template<class T, class... Args>
	auto create_camtarget(ID<Entity> entity, Args&&... args) { return create<T>(camtargets, entity, std::forward<Args>(args)...); }

	template<class T>
	auto erase_camtarget(ID<T> id) { return erase(camtargets, id); }

	// collidables
	template<class... Args>
	auto create_collidable(ID<Entity> entity, Args&&... args) { return create<Collidable>(collidables, entity, std::forward<Args>(args)...); }

	auto erase_collidable(ID<Collidable> id) { return erase(collidables, id); }

	// colliders
	template<class T, class... Args>
	auto create_collider(ID<Entity> entity, Args&&... args) { return create<T>(colliders, entity, std::forward<Args>(args)...); }

	template<class T>
	auto erase_collider(ID<T> id) { return erase(colliders, id); }

	// drawables
	template<class T, class... Args>
	auto create_drawable(ID<Entity> entity, Args&&... args) { return create<T>(drawables, entity, std::forward<Args>(args)...); }

	template<class T>
	auto erase_drawable(ID<T> id) { return erase(drawables, id); }

	// triggers
	template<class... Args>
	auto create_trigger(ID<Entity> entity, Args&&... args) { return create<Trigger>(triggers, entity, std::forward<Args>(args)...); }

	auto erase_trigger(ID<Trigger> id) { return erase(triggers, id); }

	// component spans
	auto span_camtargets() { return std::span{ camtargets.begin(), camtargets.end() }; };
	auto span_collidables() { return std::span{ collidables.begin(), collidables.end() }; };
	auto span_colliders() { return std::span{ colliders.begin(), colliders.end() }; };
	auto span_drawables() { return std::span{ drawables.begin(), drawables.end() }; };
	auto span_triggers() { return std::span{ triggers.begin(), triggers.end() }; };

	// subscribers
	void push_subscriber(WorldStateSubcriber* sub);
	void remove_subscriber(WorldStateSubcriber* sub);

	// cleanup orphan components
	void destroy_orphans();

private:
	void adopt(ID<Entity> id, GenericID component);
	//void orphan(ID<Entity> id, GenericID component);

	template<class T, class Base, bool IsPoly, class... Args>
		requires std::derived_from<T, Base>
	ID<T> create(ComponentList<Base, IsPoly>& components, ID<Entity> entity, Args&&... args)
	{
		ID<T> id;
		if constexpr (IsPoly) {
			id = components.create<T>(std::forward<Args>(args)...);
		}
		else {
			id = components.create(std::forward<Args>(args)...);
		}

		GenericID gid = id;
		adopt(entity, gid);
		for (auto& sub : subscribers)
		{
			sub->notify_component_created(gid);
		}
		return id;
	}

	template<class T, class Base, class... Args, bool IsPoly>
		requires std::derived_from<T, Base>
	bool erase(ComponentList<Base, IsPoly>& components, ID<T> id)
	{
		bool erased = components.erase(id);
		if (erased) {
			GenericID gid = id;
			for (auto& sub : subscribers)
			{
				sub->notify_component_destroyed(gid);
			}
		}
		return erased;
	}

	template<class T, class Base, bool IsPoly>
		requires std::derived_from<T, Base>
	bool exists(ComponentList<Base, IsPoly>& components, ID<T> id)
	{
		return components.exists(id);
	}

	template<class T, class Base, bool IsPoly>
		requires std::derived_from<T, Base>
	T& at(ComponentList<Base, IsPoly>& components, ID<T> id)
	{
		return components.at(id);
	}

};



}
