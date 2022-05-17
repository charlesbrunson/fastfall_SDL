#include "fastfall/game/trigger/Trigger.hpp"

#include "fastfall/game/InstanceInterface.hpp"

namespace ff {

void Trigger::set_owning_object(GameObject* object) {
	owner = object;
}

void Trigger::set_trigger_callback(TriggerFn&& trigger_fn) {
	on_trigger = std::move(trigger_fn);
}

std::optional<TriggerPull> Trigger::triggerable_by(const Trigger& trigger, secs delta_time) {
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

	auto driver_iter = drivers.find(&trigger);

	std::optional<TriggerPull> pull = std::nullopt;
	if (result && is_enabled()) {
		if (driver_iter != drivers.end()) {

			driver_iter->second.duration.delta = delta_time;
			driver_iter->second.duration.time += delta_time;
			driver_iter->second.duration.ticks++;

			pull = TriggerPull{
				.duration = driver_iter->second.duration,
				.state = State::Loop,
				.trigger = &trigger
			};
		}
		else {

			// start
			driver_iter = drivers.emplace(&trigger, TriggerData{}).first;
			driver_iter->second.duration.delta = delta_time;

			pull = TriggerPull{
				.duration = driver_iter->second.duration,
				.state = State::Entry,
				.trigger = &trigger
			};
		}
	}
	else if (driver_iter != drivers.end()) {
		// exit
		driver_iter->second.duration.delta = delta_time;
		pull = TriggerPull{
			.duration = driver_iter->second.duration,
			.state = State::Exit,
			.trigger = &trigger
		};
		drivers.erase(driver_iter);
	}
	
	return pull;
	
}

void Trigger::update(Rectf t_area) {
	area = t_area;
	activated = false;
}
void Trigger::update() {
	activated = false;
}

void Trigger::trigger(const TriggerPull& confirm) {
	if (on_trigger) {
		activated = true;
		on_trigger(confirm);
	}
}

const TriggerTag ttag_generic = "generic";
const TriggerTag ttag_hitbox  = "hitbox";
const TriggerTag ttag_hurtbox = "hurtbox";
const TriggerTag ttag_pushbox = "pushbox";

}
