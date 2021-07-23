#pragma once

#include "ext/plf_colony.h"

#include "trigger/Trigger.hpp"

#include <set>

namespace ff {

class TriggerManager {
private:


	struct Driver {

		Driver() = default;
		Driver(Trigger* t) : driver(t) {};

		mutable Trigger::Duration duration;

		Trigger* driver = nullptr;

		friend constexpr bool operator< (const Driver& lhs, const Driver& rhs) {
			return lhs.driver < rhs.driver;
		}
		friend constexpr bool operator== (const Driver& lhs, const Driver& rhs) {
			return lhs.driver == rhs.driver;
		}

	};

	struct TriggerData {
		Trigger trigger;
		std::set<Driver> drivers;
	};

public:

	TriggerManager(unsigned instance);

	Trigger* create_trigger();
	bool erase_trigger(Trigger* trigger);

	void update(secs deltaTime);

	const plf::colony<TriggerData>& get_triggers() { return triggers; };

private:


	void compareTriggers(TriggerData& A, TriggerData& B, secs deltaTime);

	plf::colony<TriggerData> triggers;

	unsigned instanceID;

};

}