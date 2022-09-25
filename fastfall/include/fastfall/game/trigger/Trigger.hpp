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

class World;
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
	using TriggerFn = std::function<void(World& w, Trigger& source, const TriggerPull&)>;

    Trigger(ID<Trigger> t_id);

    //void set_id(ID<Trigger> t_id) { m_id = t_id; }
	void set_owning_object(std::optional<ID<GameObject>> id) { owner = id; }
	void set_trigger_callback(TriggerFn&& trigger_fn);

	std::optional<TriggerPull> triggerable_by(const Trigger& trigger, secs delta_time);
	void trigger(World& w, const TriggerPull& confirm);

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

    std::optional<ID<GameObject>> get_owner_id() const { return owner; }
    GameObject* get_owner(World& w) const;

private:
	bool enabled = true;
	bool activated = false;

	Rectf area;
    std::optional<ID<GameObject>> owner;
    ID<Trigger> m_id;
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
