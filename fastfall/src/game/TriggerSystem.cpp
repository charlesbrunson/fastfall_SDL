#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/render/DebugDraw.hpp"


namespace ff {


void debugDrawTrigger(const Trigger& tr) {

	auto& varr = createDebugDrawable<VertexArray, debug_draw::Type::TRIGGER_AREA>(Primitive::TRIANGLE_STRIP, 4);
	varr[0].pos = math::rect_topleft( tr.get_area());
	varr[1].pos = math::rect_topright(tr.get_area());
	varr[2].pos = math::rect_botleft(tr.get_area());
	varr[3].pos = math::rect_botright(tr.get_area());

	Color c = tr.is_enabled() ? Color::Yellow : Color::Black;
	c.alpha(tr.is_activated() ? 200.f : 50.f);

	for (int i = 0; i < 4; i++) {
		varr[i].color = c;
	}
}

TriggerSystem::TriggerSystem()
{

}

trigger_id TriggerSystem::create() {
	return { triggers.emplace_back() };
}

trigger_id TriggerSystem::create(
	Rectf area, 
	std::unordered_set<TriggerTag> self_flags, 
	std::unordered_set<TriggerTag> filter_flags,
	GameObject* owner,
	Trigger::Overlap overlap) 
{
	auto key = triggers.emplace_back();
	auto& trigger = triggers.at(key);
	trigger.self_flags = self_flags;
	trigger.filter_flags = filter_flags;
	trigger.overlap = overlap;
	trigger.set_owning_object(owner);
	trigger.set_area(area);
	return { key };
}

bool TriggerSystem::erase(trigger_id trigger) 
{
	return triggers.erase(trigger.value) != triggers.end();
}

void TriggerSystem::update(secs deltaTime) {

	for (auto& trigger : triggers)
	{
		trigger.update();
	}
	
	if (deltaTime > 0.f && triggers.size() > 1) {
		for (auto it1 = triggers.begin(); it1 != (--triggers.end()); it1++) {
			auto it2 = it1; it2++;
			for (; it2 != triggers.end(); it2++) {

				compareTriggers(*it1, *it2, deltaTime);
				compareTriggers(*it2, *it1, deltaTime);
				

			}
		}
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::TRIGGER_AREA)) {
		for (auto& tr : triggers) {
			debugDrawTrigger(tr);
		}
	}
}

void TriggerSystem::compareTriggers(Trigger& A, Trigger& B, secs deltaTime) {

	if (auto pull = A.triggerable_by(B, deltaTime)) {
		A.trigger(pull.value());
	}
}

}
