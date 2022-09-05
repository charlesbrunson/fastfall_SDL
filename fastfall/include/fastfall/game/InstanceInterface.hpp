#pragma once

#include "GameContext.hpp"

#include <optional>
#include <string>
#include <map>

#include "fastfall/game/World.hpp"

namespace ff::instance 
{
	// object
	const ObjectSystem* obj_get_man(GameContext context);

	GameObject* obj_add(GameContext context, std::unique_ptr<GameObject>&& obj);
	GameObject* obj_get_by_level_id(GameContext context, ObjLevelID levelID);
	GameObject* obj_get_by_spawn_id(GameContext context, ObjSpawnID spawnID);

	ObjSpawnID obj_reserve_spawn_id(GameContext context);

	// collision
	const CollisionSystem* phys_get_man(GameContext context);

	const ColliderRegion* phys_get_region(GameContext context, ColliderID collider_id) noexcept;
	const ColliderRegion* phys_get_region(GameContext context, const PersistantContact& contact) noexcept;

	const ColliderQuad* phys_get_quad(GameContext context, ColliderID collider_id, int quad_id) noexcept;
	const ColliderQuad* phys_get_quad(GameContext context, const PersistantContact& contact) noexcept;

	const CollisionSystem::regions_vector* phys_get_colliders(GameContext context);
	const CollisionSystem::collidables_vector* phys_get_collidables(GameContext context);

	Collidable* phys_create_collidable(GameContext context, Vec2f init_pos, Vec2f init_size, Vec2f init_grav = Vec2f{});
	bool phys_erase_collidable(GameContext context, Collidable* collidable);

	template<ColliderType T, typename ... Args>
	T* phys_create_collider(GameContext context, Args&&... args) {
		if (context.valid())
			return Instance(context.getID())->collisions.create_collider<T>(args...);
		else {
			return nullptr;
		}
	}
	bool phys_erase_collider(GameContext context, ColliderRegion* region);

	// level
	const Level* lvl_get_active(GameContext context);
	const std::map<const std::string*, std::unique_ptr<Level>>* lvl_get_all(GameContext context);

	// camera
	const CameraSystem* cam_get_man(GameContext context);

	template<class T, class ... Args>
	ID<CameraTarget> cam_create_target(GameContext context, Args&&... args)
	{
		if (context.valid())
			return Instance(context.getID())->camera.targets.create<T>(std::forward<Args>(args)...);
		else {
			return {};
		}
	}

	void cam_erase_target(GameContext context, ID<CameraTarget> target);

	bool cam_exists(GameContext context, ID<CameraTarget> target);
	CameraTarget* cam_get(GameContext context, ID<CameraTarget> target);

	Vec2f cam_get_interpolated_pos(GameContext context, float interp);
	Vec2f cam_get_pos(GameContext context);
	void cam_set_pos(GameContext context, Vec2f pos);

	float cam_get_zoom(GameContext context);
	void cam_set_zoom(GameContext context, float zoom);

	bool cam_get_lock_enabled(GameContext context);
	void cam_set_lock_enabled(GameContext context, bool enabled);

	// trigger
	const TriggerSystem* trig_get_man(GameContext context);

	trigger_id trig_create_trigger(GameContext context);
	trigger_id trig_create_trigger(
		GameContext context,
		Rectf area,
		std::unordered_set<TriggerTag> self_flags = {},
		std::unordered_set<TriggerTag> filter_flags = {},
		GameObject* owner = nullptr,
		Trigger::Overlap overlap = Trigger::Overlap::Partial
	);
	bool trig_erase_trigger(GameContext context, trigger_id trigger);

	bool trig_exists(GameContext context, trigger_id trigger);
	Trigger* trig_get(GameContext context, trigger_id trigger);

	// scene
	const SceneSystem* scene_get_man(GameContext context);


	template<class T, class ... Args>
		requires std::derived_from<T, Drawable>
	scene_id scene_create(GameContext context, scene_config cfg, Args&&... args)
	{
		if (context.valid())
			return Instance(context.getID())->scene.create<T>(cfg, std::forward<Args>(args)...);
		else {
			return {};
		}
	}

	bool scene_erase(GameContext context, scene_id scene);

	bool scene_exists(GameContext context, scene_id scene);
	Drawable* scene_get(GameContext context, scene_id scene);

	scene_config scene_get_config(GameContext context, scene_id scene);
	void scene_set_config(GameContext context, scene_id scene, scene_config cfg);

}
