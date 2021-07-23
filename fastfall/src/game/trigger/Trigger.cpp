#include "fastfall/game/trigger/Trigger.hpp"

namespace ff {

void Trigger::set_owning_object(std::optional<GameObject*> object) {
	owner = object;
}

void Trigger::set_trigger_callback(TriggerFn&& trigger_fn) {
	on_trigger = std::move(trigger_fn);
}

Trigger::TriggerResult Trigger::triggerable_by(const Trigger& trigger) {
	Rectf intersection;
	area.intersects(trigger.area, intersection);

	bool result = false;
	for (auto tag : trigger.self_flags) {
		if (filter_flags.contains(tag)) {
			result = true;
			break;
		}
	}

	if (result) {
		switch (overlap) {
		case TriggerOverlap::Partial:
			result = intersection.width > 0.f && intersection.height > 0.f;
			break;
		case TriggerOverlap::Outside:
			result = intersection.width == 0.f && intersection.height == 0.f;
			break;
		case TriggerOverlap::Inside:
			result = intersection == area || intersection == trigger.area;
			break;
		}
	}


	return TriggerResult{ .canTrigger = result, .trigger = &trigger };
}

void Trigger::update(Rectf t_area) {
	area = t_area;
	activated = false;
}
void Trigger::update() {
	activated = false;
}

void Trigger::trigger(const Trigger::TriggerResult& confirm, const Duration& duration, TriggerState state) {
	if ((confirm.canTrigger || state == TriggerState::Exit) && on_trigger) {
		activated = true;
		on_trigger(*confirm.trigger, duration, state);
	}
}

const TriggerTag ttag_generic = "generic";
const TriggerTag ttag_hitbox  = "hitbox";
const TriggerTag ttag_hurtbox = "hurtbox";
const TriggerTag ttag_pushbox = "pushbox";

}