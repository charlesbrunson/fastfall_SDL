#include "fastfall/game/trigger/Trigger.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/object/Object.hpp"

namespace ff {

void imgui_component(const Trigger& cmp) {

    bool enabled = cmp.is_enabled();
    bool active = cmp.is_activated();
    Rectf area = cmp.get_area();

    auto& overlap = cmp.overlap;
    auto& drivers = cmp.drivers;
    auto& self_flags = cmp.self_flags;
    auto& filter_flags = cmp.filter_flags;

    ImGui::Text("Enabled: %s", enabled ? "Yes" : "No");
    ImGui::Text("Active:  %s", active  ? "Yes" : "No");
    ImGui::Text("Area: (%3.2f, %3.2f, %3.2f, %3.2f)",
                area.left, area.top, area.width, area.height);


    static constexpr std::string_view State_str[] = {
        "None",
        "Loop",
        "Entry",
        "Exit"
    };
    static constexpr std::string_view  Overlap_str[] = {
        "Partial",
        "Outside",
        "Inside"
    };

    ImGui::Text("Overlap: %s", Overlap_str[static_cast<unsigned>(overlap)].data());
    if (ImGui::TreeNode((void*)(&self_flags), "%s %d", "Self tags:", (unsigned)self_flags.size())) {
        for (auto &tag: self_flags) {
            ImGui::Text("%s", tag.tag_name_str().data());
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode((void*)(&filter_flags), "%s %d", "Filter tags:", (unsigned)filter_flags.size())) {
        for (auto &tag: filter_flags) {
            ImGui::Text("%s", tag.tag_name_str().data());
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode((void*)(&drivers), "Drivers: %d", (unsigned)drivers.size())) {
        for (auto &[id, driver]: drivers) {
            ImGui::Text("Trigger: %d:%d", id.value.sparse_index, id.value.generation);
            ImGui::Text("Duration: %f", driver.duration.time);
        }
        ImGui::TreePop();
    }
}

Trigger::Trigger(ID<Trigger> t_id)
    : m_id(t_id)
{
}

void Trigger::set_trigger_callback(TriggerFn&& trigger_fn) {
	on_trigger = std::move(trigger_fn);
}

std::optional<TriggerPull> Trigger::triggerable_by(Trigger& trigger, secs delta_time) {
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

	auto driver_iter = drivers.find(trigger.m_id);

	std::optional<TriggerPull> pull = std::nullopt;
	if (result && is_enabled()) {
		if (driver_iter != drivers.end()) {

			driver_iter->second.duration.delta = delta_time;
			driver_iter->second.duration.time += delta_time;
			driver_iter->second.duration.ticks++;

			pull = TriggerPull{
                .self = this,
                .source = &trigger,
                .state = State::Loop,
				.duration = driver_iter->second.duration,
			};
		}
		else {

			// start
			auto [iter, inserted] = drivers.emplace(trigger.m_id, TriggerData{});
            driver_iter = iter;
			driver_iter->second.duration.delta = delta_time;

			pull = TriggerPull{
                .self = this,
                .source = &trigger,
                .state = State::Entry,
				.duration = driver_iter->second.duration,
			};
		}
	}
	else if (driver_iter != drivers.end()) {
		// exit
		driver_iter->second.duration.delta = delta_time;
		pull = TriggerPull{
            .self = this,
            .source = &trigger,
            .state = State::Exit,
			.duration = driver_iter->second.duration,
		};
		drivers.erase(driver_iter);
	}
	
	return pull;
	
}

void Trigger::update() {
	activated = false;
}

void Trigger::trigger(World& w, const TriggerPull& confirm) {
	if (on_trigger) {
		activated = true;
		on_trigger(w, confirm);
	}
}

const TriggerTag ttag_generic = "generic";
const TriggerTag ttag_hitbox  = "hitbox";
const TriggerTag ttag_hurtbox = "hurtbox";
const TriggerTag ttag_pushbox = "pushbox";

}
