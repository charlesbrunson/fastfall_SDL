#pragma once

#include "fastfall/engine/time/time.hpp"

#include "fastfall/engine/input/InputSource.hpp"
#include "fastfall/engine/input/InputRecord.hpp"


namespace ff {

class InputSourceRecord : public InputSource {
public:
    InputSourceRecord(const InputRecord& t_record, size_t init_position = 0);
    const std::vector<InputEvent>& get_events() const override;
    const InputRecord& get_record() const;

    bool is_complete() const { return position >= record.frame_data.size(); }
    size_t get_tick() const { return position; }
    void set_position(size_t t_position);
    void next() override { set_position(position + 1); }
private:

    void make_events(const InputFrame& frame);

    InputRecord record;
    size_t position = 0;
    std::vector<InputEvent> curr_events;
};

}