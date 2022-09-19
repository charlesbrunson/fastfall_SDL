#include "fastfall/game/GameContext.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"

// GAME

namespace ff {

	GameContext::GameContext() :
		id{}
	{

	}

	GameContext::GameContext(InstanceID instanceID) :
		id{ instanceID }
	{

	}

	bool GameContext::valid() const noexcept {
		return (id != InstanceID{}) && (Instance(id) != nullptr);
	};
	/*
	// ObjectContext
	ObjectContext::ObjectContext(InstanceID instanceID) :
		id(instanceID)
	{ }

	void ObjectContext::add(std::unique_ptr<GameObject>&& obj) {
		Instance(id)->getObject().addObject(std::move(obj));
	}

	GameObject* ObjectContext::getByID(unsigned int objectID) {

		auto& objects = Instance(id)->getObject().getObjects();
		for (auto& obj_ptr : objects) {
			if (obj_ptr && obj_ptr->getID() == objectID) {
				return obj_ptr.get();
			}
		}
		return nullptr;
	}

	GameObjectManager& ObjectContext::get() const {
		return Instance(id)->getObject();
	}
	GameObjectManager* ObjectContext::operator->() {
		return &Instance(id)->getObject();
	}

	// CollisionContext
	CollisionContext::CollisionContext(InstanceID instanceID) :
		id(instanceID)
	{ }

	CollisionManager& CollisionContext::get() const {
		return Instance(id)->getCollision();
	}
	CollisionManager* CollisionContext::operator->() {
		return &Instance(id)->getCollision();
	}

	const ColliderRegion* CollisionContext::get_region(ColliderID collider_id) const noexcept {


		for (const auto& collider_ptr : Instance(id)->getCollision().get_colliders()) {
			if (collider_ptr->get_ID().value == collider_id.value) {
				return collider_ptr.get();
			}
		}

		return nullptr;
	}

	const ColliderRegion* CollisionContext::get_region(const PersistantContact& contact) const noexcept
	{
		return get_region(contact.collider_id);
	}

	const ColliderQuad* CollisionContext::get_quad(ColliderID collider_id, int quad_id) const noexcept {
		if (auto* collider = get_region(collider_id)) {
			return collider->get_quad(quad_id);
		}
		return nullptr;
	}

	const ColliderQuad* CollisionContext::get_quad(const PersistantContact& contact) const noexcept
	{
		return get_quad(contact.collider_id, contact.quad_id);
	}

	// CameraContext
	CameraContext::CameraContext(InstanceID instanceID) :
		id(instanceID)
	{ }

	GameCamera& CameraContext::get() const {
		return Instance(id)->getCamera();
	}
	GameCamera* CameraContext::operator->() {
		return &Instance(id)->getCamera();
	}

	// LevelContext
	LevelContext::LevelContext(InstanceID instanceID) :
		id(instanceID)
	{ }

	std::optional<std::reference_wrapper<Level>> LevelContext::get_active() const {
		GameInstance* inst = Instance(id);
		if (Level* lvl = inst->getActiveLevel()) {
			return std::reference_wrapper<Level>{*lvl};
		}
		else {
			return std::nullopt;
		}
	}

	std::map<const std::string*, std::unique_ptr<Level>>& LevelContext::get_all() const {
		GameInstance* inst = Instance(id);
		return inst->getAllLevels();

	}

	// TriggerContext

	TriggerContext::TriggerContext(InstanceID instanceID) :
		id(instanceID)
	{ }


	TriggerManager& TriggerContext::get() const {
		return Instance(id)->getTrigger();
	}
	TriggerManager* TriggerContext::operator-> () {
		return &Instance(id)->getTrigger();
	}

	// SceneContext

	SceneContext::SceneContext(InstanceID instanceID) :
		id(instanceID)
	{ }


	SceneManager& SceneContext::get() const {
		return Instance(id)->getScene();
	}
	SceneManager* SceneContext::operator-> () {
		return &Instance(id)->getScene();
	}
	*/
}