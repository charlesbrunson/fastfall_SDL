#pragma once

#include "fastfall/game/InstanceInterface.hpp"

namespace ff {

struct Collidable_ptr {
	Collidable_ptr(GameContext context, Vec2f init_pos, Vec2f init_size, Vec2f init_grav = Vec2f{});
	~Collidable_ptr();

	Collidable* operator->() { return m_collidable; }
	const Collidable* operator->() const { return m_collidable; }
	Collidable& get() { return *m_collidable; };
	const Collidable& get() const { return *m_collidable; };
	GameContext context() { return m_context; };

private:
	Collidable* m_collidable;
	const GameContext m_context;
};

template<ColliderType T>
struct Collider_ptr {

	template<typename ... Args>
	Collider_ptr(GameContext context, Args&&... args) 
		: m_context(context)
		, m_collider(instance::phys_create_collider<T>(context, std::move(args...)))
	{
	}
	~Collider_ptr() {
		instance::phys_erase_collider(m_context, m_collider);
	}

	T* operator->() { return m_collider; }
	const T* operator->() const { return m_collider; }
	T& get() { return *m_collider; };
	const T& get() const { return *m_collider; };
	GameContext context() { return m_context; };

private:
	T* m_collider;
	const GameContext m_context;
};


struct Trigger_ptr {
	Trigger_ptr(GameContext context);
	Trigger_ptr(
		GameContext context,
		Rectf area,
		std::unordered_set<TriggerTag> self_flags = {},
		std::unordered_set<TriggerTag> filter_flags = {},
		GameObject* owner = nullptr,
		Trigger::Overlap overlap = Trigger::Overlap::Partial
	);
	~Trigger_ptr();

	Trigger* operator->() { return m_trigger; }
	const Trigger* operator->() const { return m_trigger; }
	Trigger& get() { return *m_trigger; };
	const Trigger& get() const { return *m_trigger; };
	GameContext context() { return m_context; };

private:
	Trigger* m_trigger;
	const GameContext m_context;
};

template<class T>
requires std::is_base_of_v<Drawable, T>
struct Scene_ptr {

	template<typename ... Args>
	Scene_ptr(
		GameContext context, 
		T t_drawable, 
		SceneType type,
		SceneSystem::Layer layer = 0, 
		SceneSystem::Priority priority = SceneSystem::Priority::Normal
	)
		: m_context(context)
		, drawable(t_drawable)
	{
		instance::scene_add(context, type, drawable, layer, priority);
	}
	~Scene_ptr() {
		instance::scene_remove(m_context, drawable);
	}

	T& operator* () { return drawable; };
	const T& operator* () const { return drawable; };
	T* operator->() { return &drawable; }
	const T* operator->() const { return &drawable; }
	T& get() { return drawable; };
	const T& get() const { return drawable; };
	GameContext context() { return m_context; };

private:
	T drawable;
	const GameContext m_context;

};

}