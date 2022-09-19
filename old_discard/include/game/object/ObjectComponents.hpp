#pragma once

#include "../InstanceInterface.hpp"

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

template<class T>
struct CamTarget_ptr {

	template<typename ... Args>
	CamTarget_ptr(GameContext context, Args&&... args)
		: m_context(context)
		, m_target(instance::cam_create_target<T>(context, std::forward<Args>(args)...))
	{
	}
	~CamTarget_ptr() {
		instance::cam_erase_target(m_context, m_target);
	}

	T& operator* () { return get(); };
	const T& operator* () const { return get(); };
	T* operator->() { return instance::cam_get(m_context, m_target); }
	const T* operator->() const { return instance::cam_get(m_context, m_target); }
	T& get() { return *(T*)instance::cam_get(m_context, m_target); };
	const T& get() const { return *(T*)instance::cam_get(m_context, m_target); };
	GameContext context() { return m_context; };

private:
	ID<CameraTarget> m_target;
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

	Trigger* operator->() { return instance::trig_get(m_context, m_trigger); }
	const Trigger* operator->() const { return instance::trig_get(m_context, m_trigger); }
	Trigger& get() { return *instance::trig_get(m_context, m_trigger); };
	const Trigger& get() const { return *instance::trig_get(m_context, m_trigger); };
	GameContext context() { return m_context; };

private:
	trigger_id m_trigger;
	const GameContext m_context;
};

template<class T>
requires std::derived_from<T, Drawable>
struct Scene_ptr {
	template<class... Args>
	Scene_ptr(
		GameContext context, 
		scene_config cfg,
		Args&&... args
	)
		: m_context(context)
		, m_scene(instance::scene_create<T>(context, cfg, std::forward<Args>(args)...))
	{
		//instance::scene_add(context, type, drawable, layer, priority);
	}
	~Scene_ptr() {
		instance::scene_erase(m_context, m_scene);
	}

	T& operator* () { return get(); };
	const T& operator* () const { return get(); };
	T* operator->() { return &get(); }
	const T* operator->() const { return &get(); }
	T& get() { return *(T*)instance::scene_get(m_context, m_scene); };
	const T& get() const { return *(T*)instance::scene_get(m_context, m_scene); };
	GameContext context() { return m_context; };

private:
	scene_id m_scene;
	const GameContext m_context;

};

}