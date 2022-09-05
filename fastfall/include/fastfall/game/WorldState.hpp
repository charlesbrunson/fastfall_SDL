#pragma once

#include "fastfall/game/ComponentList.hpp"

#include "fastfall/game/camera/CameraTarget.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/render/Drawable.hpp"
#include "fastfall/game/trigger/Trigger.hpp"


/*
namespace ff {

struct Entity {
	ID<Entity> m_id;
	std::vector<GenericID> components;
};

class WorldState
{
public:
	// components
	PolyComponentList<CameraTarget>		camtargets;
	ComponentList<Collidable>			collidables;
	PolyComponentList<ColliderRegion>	colliders;
	PolyComponentList<Drawable>			drawables;
	ComponentList<Trigger>				triggers;

	// entity registry
	ID<Entity> make_entity() const;

	template<class T>
	ID<T> adopt(ID<Entity> id, ID<T> component)
	{
		adopt(component); return component;
	}

	template<class T>
	ID<T> orphan(ID<Entity> id, ID<T> component)
	{
		orphan(component); return component;
	}

	template<class T>
	T* get(GenericID component)
	{
		if (component.type_hash == typeid(T).hash_code()) {
			if constexpr (std::derived_from<T, CameraTarget>) {
				return camtargets.at(ID<CameraTarget>(component.value));
			}
			else if constexpr (std::derived_from<T, Collidable>) {
				return collidables.at(ID<Collidable>(component.value));
			}
			else if constexpr (std::derived_from<T, ColliderRegion>) {
				return colliders.at(ID<ColliderRegion>(component.value));
			}
			else if constexpr (std::derived_from<T, Drawable>) {
				return drawables.at(ID<Drawable>(component.value));
			}
			else if constexpr (std::derived_from<T, Trigger>) {
				return triggers.at(ID<Trigger>(component.value));
			}
		}
		return nullptr;
	}

	void destroy_orphans();

private:
	void adopt(ID<Entity> id, GenericID component);
	void orphan(ID<Entity> id, GenericID component);

	std::vector<GenericID> orphan_components;
	slot_map<Entity> entities;
};

}
*/
