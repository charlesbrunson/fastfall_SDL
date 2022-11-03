#pragma once

#include "fastfall/engine/time/time.hpp"

#include <vector>
#include <chrono>

namespace ff::profiler {

    struct Duration
    {
        size_t  curr_frame;
        secs    curr_uptime;

        secs    update_time;
        secs    predraw_time;
        secs    draw_time;
        secs    imgui_time;
        secs    display_time;
        secs    sleep_time;
        secs    total_time;
    };

    Duration curr_duration;
    bool enable = false;

    struct DurationBuffer
    {
    public:
        void add_time(/*Duration d*/)
        {
#if DEBUG
            if (enable) {
                buffer.push_back(curr_duration);
                update_past();
                cycle_durations();
            }
#endif
        }

        auto begin() const { return onesec_past; };
        auto end() const { return buffer.cend(); }
        auto& back() const { return buffer.back(); };
        auto size() const { return buffer.empty() ? 0 : static_cast<size_t>(end() - begin()); }
        bool empty() const { return size() == 0; }

        const Duration& operator[] (size_t ndx) const { return begin()[ndx]; }

        constexpr static secs CYCLE_DURATION = 3.0;

    private:

        std::vector<Duration> buffer;
        std::vector<Duration>::iterator onesec_past;

        void cycle_durations()
        {
            if (!buffer.empty()
                && buffer.front().curr_uptime + CYCLE_DURATION * 2.0 <= buffer.back().curr_uptime)
            {
                // rotate [onesec_past, back] to the front, erase the rest
                // this way no reallocation occurs
                buffer.erase(
                        std::rotate(buffer.begin(), onesec_past, buffer.end()),
                        buffer.end()
                );

                // reset onesec_past
                onesec_past = buffer.begin();
            }
        }

        void update_past()
        {
            onesec_past = std::lower_bound(buffer.begin(), buffer.end(), buffer.back().curr_uptime - CYCLE_DURATION,
                                           [](const Duration& b, secs a) {
                                               return b.curr_uptime < a;
                                           });
        }
    };

    struct Timer
    {
        std::chrono::steady_clock clock;
        std::chrono::time_point<std::chrono::steady_clock> start_time;

        Timer()
        {
            start_time = clock.now();
        }

        secs elapsed() const {
            return std::chrono::duration<secs>{ clock.now() - start_time }.count();
        }

        secs reset() {
            secs time = elapsed();
            start_time = clock.now();
            return time;
        }
    };

    DurationBuffer duration_buffer;
    Timer frame_timer;
}
