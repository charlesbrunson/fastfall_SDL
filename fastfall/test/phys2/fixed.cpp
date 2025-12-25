
#include "gtest/gtest.h"
#include "fastfall/util/math.hpp"
#include "fastfall/game/phys2/fixed.hpp"

using namespace ff;

TEST(fixed, init)
{
    EXPECT_EQ(fixed32_8{}.underlying, 0);

    EXPECT_EQ(fixed32_8::create( 1, 0xFF ).underlying, 0x000001'FF);

    EXPECT_EQ(fixed32_8::create(-8388608, 0).underlying, 0x800000'00);
    EXPECT_EQ(fixed32_8::create(8388607, 0xFF).underlying, 0x7FFFFF'FF);

    EXPECT_EQ(fixed32_8::create(1, -0xFF).underlying, 0x000000'01);
    EXPECT_EQ(fixed32_8::create(-1, 0xFF).underlying, 0xFFFFFF'FF);
    EXPECT_EQ(fixed32_8::create(0, -0xFF).underlying, 0xFFFFFF'01);

    EXPECT_EQ(fixed32_8::from_float(0.5f).underlying, 0x000000'80);

    EXPECT_EQ(fixed32_8::from_raw(0x12345678).underlying, 0x123456'78);
}

TEST(fixed, float_conv)
{
    EXPECT_EQ(static_cast<float>(fixed32_8::create(0)), 0.f);
    EXPECT_EQ(static_cast<float>(fixed32_8::create((1 << 23) - 1)), 8388607.f);
    EXPECT_EQ(static_cast<float>(fixed32_8::create(0, 0xFF)), 0.99609375f);
    EXPECT_EQ(static_cast<float>(fixed32_8::create(0, 0x01)), 0.00390625f);
    EXPECT_EQ(static_cast<float>(fixed32_8::create((1 << 23) - 1, 0xFF)), 8388607.99609375f);
}

TEST(fixed, mathops)
{
    EXPECT_EQ(
        fixed32_8::create(1).underlying,
        (-fixed32_8::create(-1)).underlying
    );

    EXPECT_EQ(
        (-fixed32_8::create(1)).underlying,
        (fixed32_8::create(-1)).underlying
    );


    EXPECT_EQ( 2_fx *  2_fx,  4_fx);
    EXPECT_EQ(-2_fx *  2_fx, -4_fx);
    EXPECT_EQ( 2_fx * -2_fx, -4_fx);
    EXPECT_EQ(-2_fx * -2_fx,  4_fx);

    EXPECT_EQ(100_fx / 2_fx, 50_fx);
    EXPECT_EQ(0.2_fx / 2_fx, 0.1_fx);
}

TEST(fixed, cmp)
{
    EXPECT_EQ(fixed32_8::create(1), fixed32_8::from_float(1.f));

    EXPECT_NEAR(
        static_cast<double>(fixed32_8::from_float(std::numbers::pi_v<double>)),
        std::numbers::pi_v<double>,
        1.0 / 256.0);
}

TEST(fixed, format)
{
    auto value = fixed32_8::create(15, 0x80);
    EXPECT_EQ(fmt::format("{}", value), "15.5");
    EXPECT_EQ(fmt::format("{:.3f}", value), "15.500");
}