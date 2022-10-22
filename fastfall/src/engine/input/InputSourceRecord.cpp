
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
    if (position != t_position || curr_events.empty()) {
        position = t_position;

        curr_events.clear();

        LOG_INFO("{} {}", position, record.frame_data.size());
        if (position < record.frame_data.size()) {
            //prev_frame = position == 0 ? std::nullopt : std::make_optional( record.frame_data.at(position - 1) );
            auto &frame = record.frame_data.at(position);
            make_events(
                    position == 0 ? nullptr : &record.frame_data.at(position - 1),
                    frame);
        }
        else if (position == record.frame_data.size() && !record.frame_data.empty()) {
            // deactivate all inputs
            make_events(&record.frame_data.back(), {});
        }
    }
}

void InputSourceRecord::make_events(InputFrame* prev_frame, const InputFrame& frame) {

    /*
    LOG_INFO(
        "{:08b} {:3d} {:3d} {:3d} {:3d} {:3d} {:3d} {:3d}",
        frame.pressed.to_ulong(),
        frame.magnitudes[0],
        frame.magnitudes[1],
        frame.magnitudes[2],
        frame.magnitudes[3],
        frame.magnitudes[4],
        frame.magnitudes[5],
        frame.magnitudes[6]
    );
    */

    for (size_t ndx = 0; ndx < INPUT_COUNT; ++ndx) {

        auto type = static_cast<InputType>(ndx);
        bool pressed = frame.pressed.test(ndx);
        uint8_t mag = frame.magnitudes.at(ndx);

        bool can_switch = true;

        if (prev_frame) {
            uint8_t prev_mag = prev_frame->magnitudes.at(ndx);
            can_switch = (prev_mag == 0) != (mag == 0);
        }

        if (pressed && mag == 0)
        {
            curr_events.push_back(InputEvent{
                    .type                   = type,
                    .activate_or_deactivate = true,
                    .magnitude              = 0xFF
            });
        }

        curr_events.push_back(InputEvent{
                .type                   = type,
                .activate_or_deactivate = can_switch,
                .magnitude              = mag
        });
    }
}

const std::vector<InputEvent>& InputSourceRecord::get_events() const {
    return curr_events;
}

const InputRecord& InputSourceRecord::get_record() const {
    return record;
}