#pragma once

//#include "ext/plf_colony.h"

#include "fastfall/game/trigger/Trigger.hpp"
#include "fastfall/util/slot_map.hpp"

#include <set>


namespace ff {

struct trigger_id {
	slot_key value;
	bool operator==(const trigger_id& other) const { return value == other.value; };
	bool operator!=(const trigger_id& other) const { return value != other.value; };
};

class TriggerSystem {
public:
	TriggerSystem();

	trigger_id create();
	trigger_id create(
		Rectf area, 
		std::unordered_set<TriggerTag> self_flags = {}, 
		std::unordered_set<TriggerTag> filter_flags = {},
		GameObject* owner = nullptr,
		Trigger::Overlap overlap = Trigger::Overlap::Partial
	);
	bool erase(trigger_id trigger);

	void update(secs deltaTime);

	const slot_map<Trigger>& get_triggers() { return triggers; };

	Trigger* get(trigger_id trigger) {
		return triggers.exists(trigger.value) ? &triggers.at(trigger.value) : nullptr;
	}

	bool exists(trigger_id trigger) const {
		return triggers.exists(trigger.value);
	}

private:

	void compareTriggers(Trigger& A, Trigger& B, secs deltaTime);

	slot_map<Trigger> triggers;
};

}
