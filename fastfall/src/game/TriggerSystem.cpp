#include "fastfall/game/TriggerSystem.hpp"

#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/game/World.hpp"


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

void TriggerSystem::update(secs deltaTime)
{

    auto triggers = world->all<Trigger>();
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

void TriggerSystem::compareTriggers(Trigger& A, Trigger& B, secs deltaTime)
{
	if (auto pull = A.triggerable_by(B, deltaTime)) {
		A.trigger(pull.value());
	}
}

void TriggerSystem::notify_created(ID<Trigger> id)
{
}

void TriggerSystem::notify_erased(ID<Trigger> id)
{
    // if the trigger is being erased, try to trigger any drivers associated first
    for (auto& trigger : world->all<Trigger>()) {
        auto iter = trigger.drivers.find(id);
        if (iter != trigger.drivers.end())
        {
            if (trigger.is_enabled()) {
                auto pull = TriggerPull{
                        .duration = iter->second.duration,
                        .state = Trigger::State::Exit,
                        .trigger = id
                };
                trigger.trigger(pull);
            }
            trigger.drivers.erase(iter);
        }
    }
}

}
