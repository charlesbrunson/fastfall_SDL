
#include "fastfall/engine/input/InputSourceRecord.hpp"
#include "fastfall/util/log.hpp"

using namespace ff;

InputSourceRecord::InputSourceRecord(const InputRecord& t_record, size_t init_position)
    : InputSource(t_record.listening)
    , record(t_record)
    , position(init_position)
{
    set_position(position);
}


void InputSourceRecord::set_position(size_t t_position)
{
    position = t_position;
    curr_events.clear();

    if (position < record.frame_data.size()) {
        //LOG_INFO("playback frame {} of {}", position + 1, record.frame_data.size());
        make_events(record.frame_data.at(position));
    }
    /*
    // end frame to clear active inputs
    else if (position == record.frame_data.size() && !record.frame_data.empty()) {
        //LOG_INFO("playback end");
        make_events({});
    }
    */
}

void InputSourceRecord::make_events(const InputFrame& frame)
{
    LOG_INFO(
        "playback {:5d} {:08b} {:08b} {:3d} {:3d} {:3d} {:3d} {:3d} {:3d} {:3d}",
        position,
        frame.pressed.to_ulong(),
        frame.activation_change.to_ulong(),
        frame.magnitudes[0],
        frame.magnitudes[1],
        frame.magnitudes[2],
        frame.magnitudes[3],
        frame.magnitudes[4],
        frame.magnitudes[5],
        frame.magnitudes[6]
    );

    for (size_t ndx = 0; ndx < INPUT_COUNT; ++ndx)
    {
        auto type       = static_cast<InputType>(ndx);
        bool pressed    = frame.pressed.test(ndx);
        uint8_t mag     = frame.magnitudes.at(ndx);
        bool can_switch = frame.activation_change.test(ndx);

        if (pressed && mag == 0)
        {
            curr_events.push_back(InputEvent{
                .type                   = type,
                .activate_or_deactivate = true,
                .magnitude              = 0xFF
            });
        }

        if (can_switch || mag != 0)
        {
            curr_events.push_back(InputEvent{
                .type                   = type,
                .activate_or_deactivate = can_switch,
                .magnitude              = mag
            });
        }
    }
}

const std::vector<InputEvent>& InputSourceRecord::get_events() const {
    return curr_events;
}

const InputRecord& InputSourceRecord::get_record() const {
    return record;
}