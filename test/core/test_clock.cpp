#include "gtest/gtest.h"

#include "core/clock.hpp"

#include <chrono>
#include <numeric>
#include <ranges>

struct test_chrono_clock {
    using rep        = std::chrono::steady_clock::rep;
    using period     = std::chrono::steady_clock::period;
    using duration   = std::chrono::steady_clock::duration;
    using time_point = std::chrono::steady_clock::time_point;
    static const bool is_steady = true;

    static time_point current_now;

    static time_point now() { return current_now; }

    static void set_time(auto duration) {
        test_chrono_clock::current_now = test_chrono_clock::time_point(duration_cast<test_chrono_clock::duration>(duration));
    }
};

test_chrono_clock::time_point test_chrono_clock::current_now;

TEST(test_clock, on_sync)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    using clock_t = ff::clock<test_chrono_clock>;

    constexpr int ups = 60;
    constexpr int fps = 60;

    using frames = std::chrono::duration<double, std::ratio<1, fps>>;

    test_chrono_clock::set_time(frames(0));
    clock_t clock(ups, fps);

    auto err = 0.00001;
    auto tick_duration = duration_cast<duration<double>>(frames(1)).count();

    unsigned update_counter = 0;
    unsigned draw_counter   = 0;
    for (int frame_count : std::views::iota(1, fps + 1)) {
        test_chrono_clock::set_time(frames(frame_count));
        auto tick = clock.tick();
        EXPECT_NEAR(tick.deltatime, tick_duration, err);
        EXPECT_EQ(tick.update_count, 1);
        EXPECT_NEAR(tick.interp, 1.0, err);
        update_counter += tick.update_count;
        ++draw_counter;
    }
    EXPECT_EQ(update_counter, ups);
    EXPECT_EQ(draw_counter, fps);
}

TEST(test_clock, off_sync)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    using clock_t = ff::clock<test_chrono_clock>;

    constexpr int ups = 60;
    constexpr int fps = 144;

    using frames = std::chrono::duration<double, std::ratio<1, fps>>;

    test_chrono_clock::set_time(frames(0));
    clock_t clock(ups, fps);

    auto err = 0.00001;
    auto tick_duration = duration_cast<duration<double>>(frames(1)).count();

    unsigned update_counter = 0;
    unsigned draw_counter   = 0;
    for (int frame_count : std::views::iota(1, fps + 1)) {
        test_chrono_clock::set_time(frames(frame_count));
        auto tick = clock.tick();
        EXPECT_NEAR(tick.deltatime, tick_duration, err);
        update_counter += tick.update_count;
        ++draw_counter;
    }
    EXPECT_EQ(update_counter, ups);
    EXPECT_EQ(draw_counter, fps);
}

TEST(test_clock, skip_render)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    using clock_t = ff::clock<test_chrono_clock>;

    constexpr int ups = 120;
    constexpr int fps = 60;

    using frames = std::chrono::duration<double, std::ratio<1, fps>>;

    test_chrono_clock::set_time(frames(0));
    clock_t clock(ups, fps);

    auto err = 0.00001;
    auto tick_duration = duration_cast<duration<double>>(frames(1)).count();

    unsigned update_counter = 0;
    unsigned draw_counter   = 0;
    for (int frame_count : std::views::iota(1, fps + 1)) {
        test_chrono_clock::set_time(frames(frame_count));
        auto tick = clock.tick();
        EXPECT_NEAR(tick.deltatime, tick_duration, err);
        EXPECT_EQ(tick.update_count, (update_counter % 2 == 1 ? 1 : 2));
        EXPECT_NEAR(tick.interp, (update_counter % 2 == 1 ? 0.5 : 0.0), err);
        update_counter += tick.update_count;
        ++draw_counter;
    }
    EXPECT_EQ(update_counter, ups);
    EXPECT_EQ(draw_counter, fps);
}

TEST(test_clock, slow_down)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    using clock_t = ff::clock<test_chrono_clock>;

    constexpr int ups = 300;
    constexpr int fps = 1;

    using frames = std::chrono::duration<double, std::ratio<1, fps>>;

    test_chrono_clock::set_time(frames(0));
    clock_t clock(ups, fps);

    auto err = 0.00001;
    auto tick_duration = duration_cast<duration<double>>(frames(1)).count();

    unsigned update_counter = 0;
    unsigned draw_counter   = 0;
    for (int frame_count : std::views::iota(1, fps + 1)) {
        test_chrono_clock::set_time(frames(frame_count));
        auto tick = clock.tick();
        EXPECT_NEAR(tick.deltatime, tick_duration, err);
        update_counter += tick.update_count;
        ++draw_counter;
    }
    EXPECT_EQ(update_counter, fps * 10);
    EXPECT_EQ(draw_counter, fps);
}
