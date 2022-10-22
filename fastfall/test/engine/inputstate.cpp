#include "gtest/gtest.h"

#include "fastfall/engine/input/InputState.hpp"
#include "fastfall/engine/InputConfig.hpp"

using namespace ff;

constexpr secs one_frame = 1.0 / 60.0;

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

TEST(inputstate, key_input)
{
    InputConfig::unbind(InputType::JUMP);
    InputState state;
    state.listen(InputType::JUMP);
    EXPECT_FALSE(state.push_event(keydown(SDLK_SPACE)));

    InputConfig::bindInput(InputType::JUMP, SDLK_SPACE);
    EXPECT_TRUE(state.push_event(keydown(SDLK_SPACE)));

    EXPECT_FALSE(state[InputType::JUMP].is_held());
    state.update(one_frame);
    EXPECT_TRUE(state[InputType::JUMP].is_held());

    EXPECT_TRUE(state.push_event(keyup(SDLK_SPACE)));
    state.update(one_frame);
    EXPECT_FALSE(state[InputType::JUMP].is_held());
}
