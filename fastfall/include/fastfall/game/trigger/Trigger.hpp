#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util.hpp"
#include "fastfall/util/tag.hpp"

#include <functional>
#include <optional>
#include <unordered_set>

namespace ff {

class GameObject;

enum class TriggerState {
	Loop,
	Entry,
	Exit
};
enum class TriggerOverlap {
	Partial,
	Outside,
	Inside
};

class Trigger {
private:
	struct TriggerResult {
		const bool canTrigger = false;
		const Trigger* trigger;
	};

public:

	struct Duration {
		secs delta_time = 0.f;
		secs total_time = 0.f;
		size_t tick = 0;
	};


	using TriggerFn = std::function<void(const Trigger&, const Duration&, TriggerState)>;

	void set_owning_object(std::optional<GameObject*> object = std::nullopt);
	void set_trigger_callback(TriggerFn&& trigger_fn);

	TriggerResult triggerable_by(const Trigger& trigger);
	void trigger(const TriggerResult& confirm, const Duration& duration, TriggerState state);

	//bool trigger(const Trigger& trigger);

	void add_duration(secs time, size_t ticks = 1);
	void reset_duration();

	void update(Rectf t_area);
	void update();

	TriggerOverlap overlap = TriggerOverlap::Partial;
	//uint8_t flags = 0;

	std::unordered_set<TriggerTag> self_flags;
	std::unordered_set<TriggerTag> filter_flags;

	bool is_activated() const { return activated; };
	Rectf get_area() const { return area; };

private:
	bool activated = false;

	Rectf area;
	std::optional<GameObject*> owner;
	TriggerFn on_trigger;
};


extern const TriggerTag ttag_generic;
extern const TriggerTag ttag_hitbox;
extern const TriggerTag ttag_hurtbox;
extern const TriggerTag ttag_pushbox;

}