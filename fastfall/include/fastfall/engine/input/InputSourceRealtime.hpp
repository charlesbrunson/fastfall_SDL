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
        InputSourceRealtime(const std::set<InputType>& accept_inputs, secs deltaTime, RecordInputs record_inputs);

        bool push_event(SDL_Event e);
        const std::vector<InputEvent>& get_events() const override;

        void record_events();
        void clear_events();

        void set_record(const InputRecord& t_record);
        const std::optional<InputRecord>& get_record() const;

    private:
        void process_axis(InputType type, AxisData& data, int16_t axis_pos, int16_t alt_axis_pos);


        secs deltaTime;

        std::optional<InputRecord> record;

        std::vector<InputEvent> events;
        std::set<InputType> listening;

        // need to store some intermediate data for analog inputs
        std::map<InputType, AxisData> axes;
    };

}