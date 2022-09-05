#include "fastfall/game/InstanceInterface.hpp"

namespace ff::instance {

	namespace {
		World* getInstance(GameContext context)
		{
			if (auto* inst = Instance(context.getID())) {
				return inst;
			}
			else {
				LOG_ERR_("Instance #{} does not exist!", context.getID());
				return nullptr;
			}
		}
	}


	// object
	const ObjectSystem* obj_get_man(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return &inst->objects;
		}
		return nullptr;
	}

	GameObject* obj_add(GameContext context, std::unique_ptr<GameObject>&& obj)
	{
		if (auto* inst = getInstance(context)) {
			GameObject* ptr = obj.get();
			inst->objects.addObject(std::move(obj));
			return ptr;
		}
		return nullptr;
	}

	GameObject* obj_get_by_level_id(GameContext context, ObjLevelID levelID)
	{
		if (auto* inst = getInstance(context)) {
			auto& objects = inst->objects.getObjects();
			for (auto& obj_ptr : objects) {
				if (obj_ptr 
					&& obj_ptr->level_data()
					&& obj_ptr->level_data()->level_id == levelID) {
					return obj_ptr.get();
				}
			}
		}
		return nullptr;
	}
	GameObject* obj_get_by_spawn_id(GameContext context, unsigned spawnID) 
	{
		if (auto* inst = getInstance(context)) {
			auto& objects = inst->objects.getObjects();
			for (auto& obj_ptr : objects) {
				if (obj_ptr && obj_ptr->spawn_id() == spawnID) {
					return obj_ptr.get();
				}
			}
		}
		return nullptr;
	}

	ObjSpawnID obj_reserve_spawn_id(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return inst->objects.getNextSpawnId();
		}
		return ObjSpawnID{};
	}

	// collision

	const CollisionSystem* phys_get_man(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return &inst->collisions;
		}
		return nullptr;
	}

	const ColliderRegion* phys_get_region(GameContext context, ColliderID collider_id) noexcept
	{
		if (auto* man = phys_get_man(context)) {
			for (const auto& collider_ptr : man->get_colliders()) {
				if (collider_ptr->get_ID().value == collider_id.value) {
					return collider_ptr.get();
				}
			}
		}
		return nullptr;
	}
	const ColliderRegion* phys_get_region(GameContext context, const PersistantContact& contact) noexcept
	{
		return phys_get_region(context, contact.collider_id);
	}
	const ColliderQuad* phys_get_quad(GameContext context, ColliderID collider_id, int quad_id) noexcept
	{
		if (auto* collider = phys_get_region(context, collider_id)) {
			return collider->get_quad(quad_id);
		}
		return nullptr;
	}
	const ColliderQuad* phys_get_quad(GameContext context, const PersistantContact& contact) noexcept
	{
		return phys_get_quad(context, contact.collider_id, contact.quad_id);
	}
	const CollisionSystem::regions_vector* phys_get_colliders(GameContext context)
	{
		if (auto* man = phys_get_man(context)) {
			return &man->get_colliders();
		}
		return nullptr;
	}
	const CollisionSystem::collidables_vector* phys_get_collidables(GameContext context)
	{
		if (auto* man = phys_get_man(context)) {
			return &man->get_collidables();
		}
		return nullptr;
	}

	Collidable* phys_create_collidable(GameContext context, Vec2f init_pos, Vec2f init_size, Vec2f init_grav)
	{
		if (auto* inst = getInstance(context)) {
			return inst->collisions.create_collidable(init_pos, init_size, init_grav);
		}
		return nullptr;
	}
	bool phys_erase_collidable(GameContext context, Collidable* collidable)
	{
		if (auto* inst = getInstance(context)) {
			return inst->collisions.erase_collidable(collidable);
		}
		return false;
	}

	bool phys_erase_collider(GameContext context, ColliderRegion* region)
	{
		if (auto* inst = getInstance(context)) {
			return inst->collisions.erase_collider(region);
		}
		return false;
	}

	// level
	const Level* lvl_get_active(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return inst->levels.getActiveLevel();
		}
		return nullptr;
	}
	const std::map<const std::string*, std::unique_ptr<Level>>* lvl_get_all(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return &inst->levels.getAllLevels();
		}
		return nullptr;
	}

	// camera
	const CameraSystem* cam_get_man(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return &inst->camera;
		}
		return nullptr;
	}

	void cam_erase_target(GameContext context, ID<CameraTarget> target) {
		if (auto* inst = getInstance(context)) {
			inst->camera.targets.erase(target);
		}
	}

	bool cam_exists(GameContext context, ID<CameraTarget> target)
	{
		if (auto* inst = getInstance(context)) {
			return inst->camera.targets.exists(target);
		}
		return false;
	}

	CameraTarget* cam_get(GameContext context, ID<CameraTarget> target)
	{
		if (auto* inst = getInstance(context)) {
			return cam_exists(context, target) ? &inst->camera.targets.at(target) : nullptr;
		}
		return nullptr;
	}

	Vec2f cam_get_interpolated_pos(GameContext context, float interp) {
		if (auto* man = cam_get_man(context)) {
			return math::lerp(man->prevPosition, man->currentPosition, interp);
		}
		return Vec2f{};
	}
	Vec2f cam_get_pos(GameContext context) {
		if (auto* man = cam_get_man(context)) {
			return man->currentPosition;
		}
		return Vec2f{};
	}
	void cam_set_pos(GameContext context, Vec2f pos) {
		if (auto* inst = getInstance(context)) {
			inst->camera.currentPosition = pos;
		}
	}
	float cam_get_zoom(GameContext context) {
		if (auto* man = cam_get_man(context)) {
			return man->zoomFactor;
		}
		return 0.f;
	}
	void cam_set_zoom(GameContext context, float zoom) {
		if (auto* inst = getInstance(context)) {
			inst->camera.zoomFactor = zoom;
		}
	}
	bool cam_get_lock_enabled(GameContext context) {
		if (auto* man = cam_get_man(context)) {
			return man->lockPosition;
		}
		return false;
	}
	void cam_set_lock_enabled(GameContext context, bool enabled) {
		if (auto* inst = getInstance(context)) {
			inst->camera.lockPosition = enabled;
		}
	}

	// trigger
	const TriggerSystem* trig_get_man(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return &inst->triggers;
		}
		return nullptr;
	}

	trigger_id trig_create_trigger(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return inst->triggers.create();
		}
		return {};
	}
	trigger_id trig_create_trigger(
		GameContext context,
		Rectf area,
		std::unordered_set<TriggerTag> self_flags,
		std::unordered_set<TriggerTag> filter_flags,
		GameObject* owner,
		Trigger::Overlap overlap) 
	{
		if (auto* inst = getInstance(context)) {
			return inst->triggers.create(
				area, self_flags, filter_flags, owner, overlap
			);
		}
		return {};
	}
	bool trig_erase_trigger(GameContext context, trigger_id trigger) {
		if (auto* inst = getInstance(context)) {
			return inst->triggers.erase(trigger);
		}
		return false;
	}

	bool trig_exists(GameContext context, trigger_id trigger)
	{
		if (auto* inst = getInstance(context)) {
			return inst->triggers.get(trigger);
		}
		return false;
	}

	Trigger* trig_get(GameContext context, trigger_id trigger)
	{
		if (auto* inst = getInstance(context)) {
			return inst->triggers.get(trigger);
		}
		return nullptr;
	}

	// scene
	const SceneSystem* scene_get_man(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return &inst->scene;
		}
		return nullptr;
	}

	bool scene_erase(GameContext context, scene_id scene)
	{
		if (auto* inst = getInstance(context)) {
			return inst->scene.erase(scene);
		}
		return false;
	}

	bool scene_exists(GameContext context, scene_id scene)
	{
		if (auto* inst = getInstance(context)) {
			return inst->scene.get(scene);
		}
		return false;
	}

	Drawable* scene_get(GameContext context, scene_id scene)
	{
		if (auto* inst = getInstance(context)) {
			return inst->scene.get(scene);
		}
		return nullptr;
	}

	scene_config scene_get_config(GameContext context, scene_id scene) 
	{
		if (auto* inst = getInstance(context)) {
			return inst->scene.get_config(scene);
		}
		return {};
	}

	void scene_set_config(GameContext context, scene_id scene, scene_config cfg) 
	{
		if (auto* inst = getInstance(context)) {
			inst->scene.set_config(scene, cfg);
		}
	}

}
