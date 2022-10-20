#include "gtest/gtest.h"

#include "fastfall/engine/input/Input.hpp"

using namespace ff;

TEST(input, input)
{
	Input state{InputType::JUMP};
	state.activate();
	EXPECT_TRUE(state.is_active());
	EXPECT_TRUE(state.is_held());
	EXPECT_TRUE(state.is_confirmable());
	EXPECT_TRUE(state.is_pressed(0.0));
	EXPECT_FALSE(state.is_confirmed());

	state.confirm_press();
	EXPECT_TRUE(state.is_active());
	EXPECT_TRUE(state.is_held());
	EXPECT_FALSE(state.is_confirmable());
	EXPECT_FALSE(state.is_pressed(0.0));
	EXPECT_TRUE(state.is_confirmed());

	state.deactivate();
	EXPECT_FALSE(state.is_active());
	EXPECT_FALSE(state.is_held());
	EXPECT_FALSE(state.is_confirmable());
	EXPECT_FALSE(state.is_pressed(0.0));
	EXPECT_TRUE(state.is_confirmed());
}

TEST(input, input_buffer)
{
	Input state{ InputType::JUMP };
	state.activate();
	state.update(1.0);
	EXPECT_TRUE(state.is_pressed(0.0));
	EXPECT_TRUE(state.is_pressed(1.0));
	state.update(1.0);
	EXPECT_FALSE(state.is_pressed(0.0));
	EXPECT_TRUE(state.is_pressed(2.0));
}

TEST(input, input_deactivated_buffer)
{
	Input state{ InputType::JUMP };
	state.activate();
	state.deactivate();
	state.update(1.0);
	EXPECT_TRUE(state.is_pressed(1.0));
}