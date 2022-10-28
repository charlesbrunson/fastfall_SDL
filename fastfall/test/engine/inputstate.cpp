#include "gtest/gtest.h"

#include "fastfall/engine/input/InputState.hpp"
#include "fastfall/engine/InputConfig.hpp"
#include "fastfall/engine/input/InputSourceRealtime.hpp"
#include "fastfall/engine/input/InputSourceRecord.hpp"

#include "fastfall/util/log.hpp"

using namespace ff;

constexpr secs one_frame = 1.0 / 60.0;

void cmp_input(const Input& lhs, const Input& rhs)
{
    assert(lhs.type() == rhs.type());
    EXPECT_EQ(lhs.is_held(),      rhs.is_held());
    EXPECT_EQ(lhs.is_confirmed(), rhs.is_confirmed());
    EXPECT_EQ(lhs.magnitude(),    rhs.magnitude());
}

SDL_Event keydown(SDL_Keycode code)
{
    SDL_Event e;
    e.key = {};
    e.key.type = SDL_KEYDOWN;
    e.key.keysym.sym = code;
    e.key.keysym.scancode = SDL_GetScancodeFromKey(code);
    e.key.repeat = 0;
    e.key.state = 0;
    return e;
}

SDL_Event keyup(SDL_Keycode code)
{
    SDL_Event e;
    e.key = {};
    e.key.type = SDL_KEYUP;
    e.key.keysym.sym = code;
    e.key.keysym.scancode = SDL_GetScancodeFromKey(code);
    e.key.repeat = 0;
    e.key.state = 0;
    return e;
}

SDL_Event axis(SDL_GameControllerAxis axis, int16_t position)
{
    SDL_Event e;
    e.caxis = {};
    e.caxis.type = SDL_CONTROLLERAXISMOTION;
    e.caxis.axis = axis;
    e.caxis.which = 0;
    e.caxis.value = position;
    return e;
}

void expect_matching_inputs(InputSourceRealtime& realtime, InputState& state_realtime, InputType type) {
    auto inputrecord = *realtime.get_record();

    InputSourceRecord record{ inputrecord };
    InputState state_record{ &record };

    while (!record.is_complete()) {
        state_record.update(one_frame);
        record.next();
    }

    auto space_real   = state_realtime.get(type);
    auto space_record = state_record.get(type);

    cmp_input(*space_real, *space_record);
}

TEST(inputstate, realtime_matches_record_source)
{
    {
        // key down
        InputSourceRealtime realtime{input_sets::gameplay, one_frame, RecordInputs::Yes};
        InputState state_realtime{&realtime};

        realtime.push_event(keydown(SDLK_SPACE));
        state_realtime.update(one_frame);
        realtime.next();

        expect_matching_inputs(realtime, state_realtime, InputType::JUMP);
    }
    LOG_INFO("--------------");
    {
        // key down and release same frame
        InputSourceRealtime realtime{input_sets::gameplay, one_frame, RecordInputs::Yes};
        InputState state_realtime{&realtime};

        realtime.push_event(keydown(SDLK_SPACE));
        realtime.push_event(keyup(SDLK_SPACE));
        state_realtime.update(one_frame);
        realtime.next();

        expect_matching_inputs(realtime, state_realtime, InputType::JUMP);
    }
    LOG_INFO("--------------");
    {
        // key down, hold, release
        InputSourceRealtime realtime{input_sets::gameplay, one_frame, RecordInputs::Yes};
        InputState state_realtime{&realtime};

        realtime.push_event(keydown(SDLK_SPACE));
        state_realtime.update(one_frame);
        realtime.next();

        state_realtime.update(one_frame);
        realtime.next();

        realtime.push_event(keyup(SDLK_SPACE));
        state_realtime.update(one_frame);
        realtime.next();

        expect_matching_inputs(realtime, state_realtime, InputType::JUMP);
    }
    LOG_INFO("--------------");
    {
        // axis right
        InputSourceRealtime realtime{input_sets::gameplay, one_frame, RecordInputs::Yes};
        InputState state_realtime{&realtime};

        realtime.push_event(axis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX, 0x7FFF));
        state_realtime.update(one_frame);
        realtime.next();

        realtime.push_event(axis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX, 0x3FFF));
        state_realtime.update(one_frame);
        realtime.next();

        realtime.push_event(axis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX, 0x0000));
        state_realtime.update(one_frame);
        realtime.next();

        realtime.push_event(axis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX, 0x7FFF));
        state_realtime.update(one_frame);
        realtime.next();

        expect_matching_inputs(realtime, state_realtime, InputType::RIGHT);
    }
}













