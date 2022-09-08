#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/id.hpp"

#include <functional>
#include <optional>
#include <unordered_set>
#include <map>

namespace ff {

class GameObject;

struct TriggerPull;

class Trigger {
public:
	enum class State {
		None,
		Loop,
		Entry,
		Exit
	};
	enum class Overlap {
		Partial,
		Outside,
		Inside
	};
	struct Duration {
		secs delta = 0.0;
		secs time = 0.0;
		size_t ticks = 0u;
	};

private:
	struct TriggerData {
		Duration duration;
		State state = State::None;
	};

public:
	using TriggerFn = std::function<void(const TriggerPull&)>;

	void set_owning_object(ID<GameObject> id); // can take nullptr
	void set_trigger_callback(TriggerFn&& trigger_fn);

	std::optional<TriggerPull> triggerable_by(const Trigger& trigger, secs delta_time);
	void trigger(const TriggerPull& confirm);

	void add_duration(secs time, size_t ticks = 1);
	void reset_duration();

	void update();

	Overlap overlap = Overlap::Partial;

	std::unordered_set<TriggerTag> self_flags;
	std::unordered_set<TriggerTag> filter_flags;

	bool is_activated() const { return activated; };
	Rectf get_area() const { return area; };
	void set_area(Rectf area_) { area = area_; };

	void set_enable(bool t_enabled) { enabled = t_enabled; };
	bool is_enabled() const { return enabled; };

	std::map<ID<Trigger>, TriggerData> drivers;

private:
	bool enabled = true;
	bool activated = false;

	Rectf area;
    ID<GameObject> owner;
	TriggerFn on_trigger;
};

struct TriggerPull {
	Trigger::Duration duration;
	Trigger::State state = Trigger::State::None;
	ID<Trigger> trigger;
};

extern const TriggerTag ttag_generic;
extern const TriggerTag ttag_hitbox;
extern const TriggerTag ttag_hurtbox;
extern const TriggerTag ttag_pushbox;



}
