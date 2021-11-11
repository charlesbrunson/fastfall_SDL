#include "fastfall/game/InstanceInterface.hpp"

namespace ff::instance {

	namespace {
		GameInstance* getInstance(GameContext context)
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
	const GameObjectManager* obj_get_man(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return &inst->getObject();
		}
		return nullptr;
	}

	GameObject* obj_add(GameContext context, std::unique_ptr<GameObject>&& obj)
	{
		if (auto* inst = getInstance(context)) {
			GameObject* ptr = obj.get();
			inst->getObject().addObject(std::move(obj));
			return ptr;
		}
		return nullptr;
	}

	GameObject* obj_get_by_level_id(GameContext context, unsigned levelID)
	{
		if (auto* inst = getInstance(context)) {
			auto& objects = inst->getObject().getObjects();
			for (auto& obj_ptr : objects) {
				if (obj_ptr 
					&& obj_ptr->getTemplate()
					&& obj_ptr->getTemplate()->level_id() == levelID) {
					return obj_ptr.get();
				}
			}
		}
		return nullptr;
	}
	GameObject* obj_get_by_spawn_id(GameContext context, unsigned spawnID) 
	{
		if (auto* inst = getInstance(context)) {
			auto& objects = inst->getObject().getObjects();
			for (auto& obj_ptr : objects) {
				if (obj_ptr && obj_ptr->getID() == spawnID) {
					return obj_ptr.get();
				}
			}
		}
		return nullptr;
	}

	unsigned obj_reserve_spawn_id(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return inst->getObject().getNextSpawnId();
		}
		return 0u;
	}

	// collision

	const CollisionManager* phys_get_man(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return &inst->getCollision();
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
	const std::vector<std::unique_ptr<ColliderRegion>>* phys_get_colliders(GameContext context)
	{
		if (auto* man = phys_get_man(context)) {
			return &man->get_colliders();
		}
		return nullptr;
	}
	const plf::colony<CollisionManager::CollidableData>* phys_get_collidables(GameContext context)
	{
		if (auto* man = phys_get_man(context)) {
			return &man->get_collidables();
		}
		return nullptr;
	}

	Collidable* phys_create_collidable(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return inst->getCollision().create_collidable();
		}
		return nullptr;
	}
	Collidable* phys_create_collidable(GameContext context, Vec2f init_pos, Vec2f init_size, Vec2f init_grav)
	{
		if (auto* inst = getInstance(context)) {
			return inst->getCollision().create_collidable(init_pos, init_size, init_grav);
		}
		return nullptr;
	}
	bool phys_erase_collidable(GameContext context, Collidable* collidable)
	{
		if (auto* inst = getInstance(context)) {
			return inst->getCollision().erase_collidable(collidable);
		}
		return false;
	}

	bool phys_erase_collider(GameContext context, ColliderRegion* region)
	{
		if (auto* inst = getInstance(context)) {
			return inst->getCollision().erase_collider(region);
		}
		return false;
	}

	// level
	const Level* lvl_get_active(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return inst->getActiveLevel();
		}
		return nullptr;
	}
	const std::map<const std::string*, std::unique_ptr<Level>>* lvl_get_all(GameContext context)
	{
		if (auto* inst = getInstance(context)) {
			return &inst->getAllLevels();
		}
		return nullptr;
	}

	// camera
	const GameCamera* cam_get_man(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return &inst->getCamera();
		}
		return nullptr;
	}

	void cam_add_target(GameContext context, CameraTarget* target) {
		if (auto* inst = getInstance(context)) {
			inst->getCamera().addTarget(target);
		}
	}
	void cam_remove_target(GameContext context, CameraTarget* target) {
		if (auto* inst = getInstance(context)) {
			inst->getCamera().removeTarget(target);
		}
	}
	Vec2f cam_get_pos(GameContext context) {
		if (auto* man = cam_get_man(context)) {
			return man->currentPosition;
		}
		return Vec2f{};
	}
	void cam_set_pos(GameContext context, Vec2f pos) {
		if (auto* inst = getInstance(context)) {
			inst->getCamera().currentPosition = pos;
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
			inst->getCamera().zoomFactor = zoom;
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
			inst->getCamera().lockPosition = enabled;
		}
	}

	// trigger
	const TriggerManager* trig_get_man(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return &inst->getTrigger();
		}
		return nullptr;
	}

	Trigger* trig_create_trigger(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return inst->getTrigger().create_trigger();
		}
		return nullptr;
	}
	Trigger* trig_create_trigger(
		GameContext context,
		Rectf area,
		std::unordered_set<TriggerTag> self_flags,
		std::unordered_set<TriggerTag> filter_flags,
		GameObject* owner,
		Trigger::Overlap overlap) 
	{
		if (auto* inst = getInstance(context)) {
			return inst->getTrigger().create_trigger(
				area, self_flags, filter_flags, owner, overlap
			);
		}
		return nullptr;
	}
	bool trig_erase_trigger(GameContext context, Trigger* trigger) {
		if (auto* inst = getInstance(context)) {
			return inst->getTrigger().erase_trigger(trigger);
		}
		return false;
	}

	// scene
	const SceneManager* scene_get_man(GameContext context) {
		if (auto* inst = getInstance(context)) {
			return &inst->getScene();
		}
		return nullptr;
	}

	void scene_add(
		GameContext context,
		SceneType scene_type,
		Drawable& drawable,
		SceneManager::Layer layer,
		SceneManager::Priority priority) 
	{
		if (auto* inst = getInstance(context)) {
			inst->getScene().add(scene_type, drawable, layer, priority);
		}
	}

	void scene_remove(GameContext context, Drawable& drawable) {

		if (auto* inst = getInstance(context)) {
			inst->getScene().remove(drawable);
		}
	}
}
