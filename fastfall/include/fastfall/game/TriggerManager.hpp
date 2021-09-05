#pragma once

#include "ext/plf_colony.h"

#include "trigger/Trigger.hpp"

#include <set>

namespace ff {

class TriggerManager {
public:
	//using Trigger_sptr = std::unique_ptr<Trigger>;

	TriggerManager(unsigned instance);

	Trigger* create_trigger();
	Trigger* create_trigger(
		Rectf area, 
		std::unordered_set<TriggerTag> self_flags = {}, 
		std::unordered_set<TriggerTag> filter_flags = {},
		GameObject* owner = nullptr,
		Trigger::Overlap overlap = Trigger::Overlap::Partial
	);
	bool erase_trigger(Trigger* trigger);

	void update(secs deltaTime);

	const plf::colony<Trigger>& get_triggers() { return triggers; };

private:

	void compareTriggers(Trigger& A, Trigger& B, secs deltaTime);

	plf::colony<Trigger> triggers;

	unsigned instanceID;

};

}
