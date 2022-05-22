#include "fastfall/game/object/ObjectComponents.hpp"

namespace ff {

Collidable_ptr::Collidable_ptr(GameContext context, Vec2f init_pos, Vec2f init_size, Vec2f init_grav)
	: m_context(context)
	, m_collidable(instance::phys_create_collidable(context, init_pos, init_size, init_grav))
{

}

Collidable_ptr::~Collidable_ptr()
{
	instance::phys_erase_collidable(m_context, m_collidable);
}

Trigger_ptr::Trigger_ptr(GameContext context)
	: m_context(context)
	, m_trigger(instance::trig_create_trigger(context))
{

}
Trigger_ptr::Trigger_ptr(
	GameContext context,
	Rectf area,
	std::unordered_set<TriggerTag> self_flags,
	std::unordered_set<TriggerTag> filter_flags,
	GameObject* owner,
	Trigger::Overlap overlap
)
	: m_context(context)
	, m_trigger(
		instance::trig_create_trigger(context, area, self_flags, filter_flags, owner, overlap)
	)
{

}
Trigger_ptr::~Trigger_ptr() {
	instance::trig_erase_trigger(m_context, m_trigger);
}


}