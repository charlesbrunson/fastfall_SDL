#pragma once

#include "fastfall/game/ComponentList.hpp"

#include "fastfall/game/Entity.hpp"
#include "fastfall/game/WorldStateListener.hpp"

#include "fastfall/game/camera/CameraTarget.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"
//#include "fastfall/render/Drawable.hpp"
//#include "fastfall/game/trigger/Trigger.hpp"

#include <span>

namespace ff {

using CamTargetList		= ComponentList<CameraTarget, true>;
using CollidableList	= ComponentList<Collidable, false>;
using ColliderList		= ComponentList<ColliderRegion, true>;
using SurfaceTrackerList = ComponentList<SurfaceTracker, false>;
//using DrawableList		= ComponentList<Drawable, true>;
//using TriggerList		= ComponentList<Trigger, false>;

class WorldState
{
private:

	CamTargetList		camtargets;
	CollidableList		collidables;
	ColliderList		colliders;
	SurfaceTrackerList	trackers;
	//DrawableList		drawables;
	//TriggerList		triggers;

	std::vector<GenericID> orphan_components;
	slot_map<Entity> entities;

	std::vector<WorldStateListener*> listeners;

public:

	// entities
	ID<Entity> make_entity();
	bool erase_entity(ID<Entity> id);

	template<class T>
	auto list() {  
		return get_list_for<T>();
	};

	// subscribers
	void push_listener(WorldStateListener* sub);
	void remove_listener(WorldStateListener* sub);

	// cleanup orphan components
	void destroy_orphans();

	template<class T, class... Args>
	ID<T> create(ID<Entity> entity, Args&&... args) 
	{
		if constexpr (decltype(get_list_for<T>())::is_poly) {
			return get_list_for<T>().create<T>(std::forward<Args>(args)...);
		}
		else {
			return get_list_for<T>().create(std::forward<Args>(args)...);
		}
	}

	template<class T>
	T& get(ID<T> id)
	{
		return get_list_for<T>().at(id);
	}

	template<class T>
	bool erase(ID<T> id) 
	{
		return get_list_for<T>().erase(id);
	}

	template<class T>
	bool exists(ID<T> id) 
	{
		return get_list_for<T>().exists(id);
	}

	template<class T>
	ID<T> id(const T& component)
	{
		return get_list_for<T>().id(component);
	}


private:
	void adopt(ID<Entity> id, GenericID component);

	template<class T>
	constexpr auto& get_list_for() 
	{
		if constexpr (std::derived_from<T, CamTargetList::type>)
		{
			return camtargets;
		}
		else if constexpr (std::derived_from<T, CollidableList::type>)
		{
			return collidables;
		}
		else if constexpr (std::derived_from<T, ColliderList::type>)
		{
			return colliders;
		}
		else if constexpr (std::derived_from<T, SurfaceTrackerList::type>)
		{
			return trackers;
		}
		/*
		else if constexpr (std::derived_from<T, DrawableList::type>)
		{
			return drawables;
		}
		else if constexpr (std::derived_from<T, TriggerList::type>)
		{
			return triggers;
		}
		*/
		else
		{
			[failed = true]() -> void {
				static_assert(failed, "whoops");
			}();
		}
	}
};



}
