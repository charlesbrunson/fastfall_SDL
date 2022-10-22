#pragma once

#include "fastfall/engine/time/time.hpp"

#include "fastfall/engine/input/InputSource.hpp"
#include "fastfall/engine/input/InputRecord.hpp"


namespace ff {

class InputSourceRecording : public InputSource {
public:
    InputSourceRecording(const InputRecord<INPUT_COUNT>& t_record, size_t init_position);
    const std::vector<InputEvent>& get_events() const override;
    const InputRecord<INPUT_COUNT>& get_record() const;
private:
    InputRecord<INPUT_COUNT> record;
    size_t position = 0;
};

}