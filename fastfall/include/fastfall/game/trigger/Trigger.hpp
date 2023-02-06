#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util.hpp"
#include "fastfall/util/tag.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/game/attach/Attachable.hpp"

#include <functional>
#include <optional>
#include <unordered_set>
#include <map>

namespace ff {

class World;
class Object;

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
	using TriggerFn = std::function<void(World& w, const TriggerPull&)>;

    Trigger(ID<Trigger> t_id);

	void set_trigger_callback(TriggerFn&& trigger_fn);
	std::optional<TriggerPull> triggerable_by(Trigger& trigger, secs delta_time);
	void trigger(World& w, const TriggerPull& confirm);

	void update();

	void set_area(Rectf area_) { area = area_; };
    void set_enable(bool t_enabled) { enabled = t_enabled; };

    ID<Trigger> id() const { return m_id; };
	bool  is_enabled() const { return enabled; };
    bool  is_activated() const { return activated; };
    Rectf get_area() const { return area; };

    Overlap overlap = Overlap::Partial;
	std::map<ID<Trigger>, TriggerData> drivers;
    std::unordered_set<TriggerTag> self_flags;
    std::unordered_set<TriggerTag> filter_flags;

private:
	bool enabled = true;
	bool activated = false;

	Rectf       area;
    ID<Trigger> m_id;
	TriggerFn   on_trigger;
};

struct TriggerPull {
    Trigger* self = nullptr;
    Trigger* source = nullptr;
    Trigger::State state = Trigger::State::None;
    Trigger::Duration duration;
};

extern const TriggerTag ttag_generic;
extern const TriggerTag ttag_hitbox;
extern const TriggerTag ttag_hurtbox;
extern const TriggerTag ttag_pushbox;

}
