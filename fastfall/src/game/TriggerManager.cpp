#include "fastfall/game/TriggerManager.hpp"
#include "fastfall/render/DebugDraw.hpp"


namespace ff {


void debugDrawTrigger(const Trigger& tr) {

	//debug_draw::set_offset(tr.area.getPosition());
	auto& varr = createDebugDrawable<VertexArray, debug_draw::Type::TRIGGER_AREA>(Primitive::TRIANGLE_STRIP, 4);
	varr[0].pos = math::rect_topleft( tr.get_area());
	varr[1].pos = math::rect_topright(tr.get_area());
	varr[2].pos = math::rect_botleft(tr.get_area());
	varr[3].pos = math::rect_botright(tr.get_area());


	Color c = Color::Yellow;
	c.alpha(tr.is_activated() ? 200.f : 50.f);

	for (int i = 0; i < 4; i++) {
		varr[i].color = c;
	}
}

TriggerManager::TriggerManager(unsigned instance) 
	: instanceID(instance)
{

}

Trigger* TriggerManager::create_trigger() {
	return &triggers.emplace()->trigger;
}
bool TriggerManager::erase_trigger(Trigger* trigger) {

	auto it = std::find_if(triggers.begin(), triggers.end(), [trigger](const TriggerData& data) {
			return trigger == &data.trigger;
		});

	if (it != triggers.end()) {
		triggers.erase(it);
	}
	return false;
}

void TriggerManager::update(secs deltaTime) {
	
	if (deltaTime > 0.f) {
		for (auto it1 = triggers.begin(); it1 != (--triggers.end()); it1++) {
			auto it2 = it1; it2++;
			for (; it2 != triggers.end(); it2++) {
				compareTriggers(*it1, *it2, deltaTime);
				compareTriggers(*it2, *it1, deltaTime);
			}
		}

		if (debug_draw::hasTypeEnabled(debug_draw::Type::TRIGGER_AREA)) {
			for (auto& tr : triggers) {
				debugDrawTrigger(tr.trigger);
			}
		}
	}

}

void TriggerManager::compareTriggers(TriggerData& A, TriggerData& B, secs deltaTime) {
	auto driver_iter = A.drivers.find(&B.trigger);

	if (auto result = A.trigger.triggerable_by(B.trigger); result.canTrigger) {
		if (driver_iter != A.drivers.end()) {

			// repeat
			driver_iter->duration.delta_time = deltaTime;
			driver_iter->duration.total_time += deltaTime;
			driver_iter->duration.tick++;
			A.trigger.trigger(result, driver_iter->duration, TriggerState::Loop);
		}
		else {

			// start
			driver_iter = A.drivers.insert(&B.trigger).first;
			driver_iter->duration.delta_time = deltaTime;
			A.trigger.trigger(result, driver_iter->duration, TriggerState::Entry);
		}
	}
	else if (driver_iter != A.drivers.end()) {

		// exit
		driver_iter->duration.delta_time = deltaTime;
		A.trigger.trigger(result, driver_iter->duration, TriggerState::Exit);
		A.drivers.erase(driver_iter);
	}
}

}