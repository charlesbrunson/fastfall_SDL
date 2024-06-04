#pragma once

#include "fastfall/engine/time/time.hpp"

#include "fastfall/engine/input/InputSource.hpp"
#include "fastfall/engine/input/InputRecord.hpp"

#include <optional>
#include <set>

#include "SDL_events.h"
#include "SDL_keycode.h"

namespace ff {
    enum class RecordInputs : bool {
        No  = false,
        Yes = true
    };

    class InputSourceRealtime : public InputSource {
    private:
        struct AxisData {
            bool prev_in_range;
            bool positive_side;
        };

    public:
        InputSourceRealtime(const std::set<Input>& accept_inputs, secs deltaTime, RecordInputs record_inputs);

        bool push_event(SDL_Event e);
        const std::vector<InputEvent>& get_events() const override;
        void next() override;

        size_t get_tick() const { return tick; }

        void set_record(const InputRecord& t_record);
        const std::optional<InputRecord>& get_record() const;
        bool is_recording() const { return record.has_value(); }

    private:
        void process_axis(Input type, AxisData& data, int16_t axis_pos, int16_t alt_axis_pos);


        secs deltaTime;
        size_t tick = 0;

        std::optional<InputRecord> record;

        std::vector<InputEvent> events;
        std::set<Input> listening;

        // need to store some intermediate data for analog inputs
        std::map<Input, AxisData> axes;
    };

}