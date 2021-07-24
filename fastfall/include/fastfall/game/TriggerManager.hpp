#pragma once

#include "ext/plf_colony.h"

#include "trigger/Trigger.hpp"

#include <set>

namespace ff {

class TriggerManager {
public:
	using Trigger_sptr = std::shared_ptr<Trigger>;

	TriggerManager(unsigned instance);

	Trigger* create_trigger();
	bool erase_trigger(Trigger* trigger);

	void update(secs deltaTime);

	const std::vector<Trigger_sptr>& get_triggers() { return triggers; };

private:

	void compareTriggers(Trigger_sptr& A, Trigger_sptr& B, secs deltaTime);

	std::vector<Trigger_sptr> triggers;

	unsigned instanceID;

};

}