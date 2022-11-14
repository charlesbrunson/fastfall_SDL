#include "fastfall/game/trigger/Trigger.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/object/GameObject.hpp"

namespace ff {

Trigger::Trigger(ID<Trigger> t_id)
    : m_id(t_id)
{
}

void Trigger::set_trigger_callback(TriggerFn&& trigger_fn) {
	on_trigger = std::move(trigger_fn);
}

std::optional<TriggerPull> Trigger::triggerable_by(Trigger& trigger, secs delta_time) {
	Rectf intersection;
	area.intersects(trigger.area, intersection);

	bool result = false;
	for (const auto& tag : trigger.self_flags) {
		if (filter_flags.contains(tag)) {
			result = true;
			break;
		}
	}

	if (result) {
		switch (overlap) {
		case Overlap::Partial:
			result = intersection.width > 0.f && intersection.height > 0.f;
			break;
		case Overlap::Outside:
			result = intersection.width == 0.f && intersection.height == 0.f;
			break;
		case Overlap::Inside:
			result = intersection == area || intersection == trigger.area;
			break;
		}
	}

	auto driver_iter = drivers.find(trigger.m_id);

	std::optional<TriggerPull> pull = std::nullopt;
	if (result && is_enabled()) {
        bool is_end = driver_iter == drivers.end();
		if (driver_iter != drivers.end()) {

			driver_iter->second.duration.delta = delta_time;
			driver_iter->second.duration.time += delta_time;
			driver_iter->second.duration.ticks++;

			pull = TriggerPull{
                .self = this,
                .source = &trigger,
                .state = State::Loop,
				.duration = driver_iter->second.duration,
			};
		}
		else {

			// start
			auto [iter, inserted] = drivers.emplace(trigger.m_id, TriggerData{});
            driver_iter = iter;
			driver_iter->second.duration.delta = delta_time;

			pull = TriggerPull{
                .self = this,
                .source = &trigger,
                .state = State::Entry,
				.duration = driver_iter->second.duration,
			};
		}
	}
	else if (driver_iter != drivers.end()) {
		// exit
		driver_iter->second.duration.delta = delta_time;
		pull = TriggerPull{
            .self = this,
            .source = &trigger,
            .state = State::Exit,
			.duration = driver_iter->second.duration,
		};
		drivers.erase(driver_iter);
	}
	
	return pull;
	
}

void Trigger::update() {
	activated = false;
}

void Trigger::trigger(World& w, const TriggerPull& confirm) {
	if (on_trigger) {
		activated = true;
		on_trigger(w, confirm);
	}
}

const TriggerTag ttag_generic = "generic";
const TriggerTag ttag_hitbox  = "hitbox";
const TriggerTag ttag_hurtbox = "hurtbox";
const TriggerTag ttag_pushbox = "pushbox";

}
