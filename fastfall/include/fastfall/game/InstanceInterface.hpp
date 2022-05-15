#pragma once

#include "GameContext.hpp"

#include <optional>
#include <string>
#include <map>

#include "fastfall/game/Instance.hpp"

// object
#include "fastfall/game/GameObjectManager.hpp"

// collision
#include "fastfall/game/CollisionManager.hpp"

// level
#include "fastfall/game/Level.hpp"

// camera
#include "fastfall/game/GameCamera.hpp"

// trigger
#include "fastfall/game/TriggerManager.hpp"

// scene
#include "fastfall/game/SceneManager.hpp"


namespace ff::instance 
{
	// object
	const GameObjectManager* obj_get_man(GameContext context);

	GameObject* obj_add(GameContext context, std::unique_ptr<GameObject>&& obj);
	GameObject* obj_get_by_level_id(GameContext context, ObjLevelID levelID);
	GameObject* obj_get_by_spawn_id(GameContext context, ObjSpawnID spawnID);

	ObjSpawnID obj_reserve_spawn_id(GameContext context);

	// collision
	const CollisionManager* phys_get_man(GameContext context);

	const ColliderRegion* phys_get_region(GameContext context, ColliderID collider_id) noexcept;
	const ColliderRegion* phys_get_region(GameContext context, const PersistantContact& contact) noexcept;

	const ColliderQuad* phys_get_quad(GameContext context, ColliderID collider_id, int quad_id) noexcept;
	const ColliderQuad* phys_get_quad(GameContext context, const PersistantContact& contact) noexcept;

	const CollisionManager::regions_vector* phys_get_colliders(GameContext context);
	const CollisionManager::collidables_vector* phys_get_collidables(GameContext context);

	Collidable* phys_create_collidable(GameContext context);
	Collidable* phys_create_collidable(GameContext context, Vec2f init_pos, Vec2f init_size, Vec2f init_grav = Vec2f{});
	bool phys_erase_collidable(GameContext context, Collidable* collidable);

	template<ColliderType T, typename ... Args>
	T* phys_create_collider(GameContext context, Args&&... args) {
		if (context.valid())
			return Instance(context.getID())->getCollision().create_collider<T>(args...);
		else {
			return nullptr;
		}
	}
	bool phys_erase_collider(GameContext context, ColliderRegion* region);

	// level
	const Level* lvl_get_active(GameContext context);
	const std::map<const std::string*, std::unique_ptr<Level>>* lvl_get_all(GameContext context);

	// camera
	const GameCamera* cam_get_man(GameContext context);

	void cam_add_target(GameContext context, CameraTarget& target);
	void cam_remove_target(GameContext context, CameraTarget& target);

	Vec2f cam_get_interpolated_pos(GameContext context, float interp);
	Vec2f cam_get_pos(GameContext context);
	void cam_set_pos(GameContext context, Vec2f pos);

	float cam_get_zoom(GameContext context);
	void cam_set_zoom(GameContext context, float zoom);

	bool cam_get_lock_enabled(GameContext context);
	void cam_set_lock_enabled(GameContext context, bool enabled);

	// trigger
	const TriggerManager* trig_get_man(GameContext context);

	Trigger* trig_create_trigger(GameContext context);
	Trigger* trig_create_trigger(
		GameContext context,
		Rectf area,
		std::unordered_set<TriggerTag> self_flags = {},
		std::unordered_set<TriggerTag> filter_flags = {},
		GameObject* owner = nullptr,
		Trigger::Overlap overlap = Trigger::Overlap::Partial
	);
	bool trig_erase_trigger(GameContext context, Trigger* trigger);


	// scene
	const SceneManager* scene_get_man(GameContext context);

	void scene_add(
		GameContext context, 
		SceneType scene_type, 
		Drawable& drawable, 
		SceneManager::Layer layer = 0, 
		SceneManager::Priority priority = SceneManager::Priority::Normal);

	void scene_remove(GameContext context, Drawable& drawable);
}
