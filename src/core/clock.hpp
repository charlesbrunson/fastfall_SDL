#pragma once

#include "ff/core/time.hpp"

#include <algorithm>
#include <chrono>

namespace ff {

// template to allow dependency injection
// (mainly for testing)
template<class ChronoClock = std::conditional<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock>::type>
class clock {
private:
    using clock_type = ChronoClock;
    using time_point = clock_type::time_point;
    using time_res = std::chrono::nanoseconds;
    using sec_rep = std::chrono::duration<double>;

public:
    constexpr static unsigned MIN_UPS       = 10;
    constexpr static unsigned FPS_UNLIMITED = 0;
    constexpr static size_t   UPDATE_MAX    = 3;

public:
    clock(unsigned ups = 60, unsigned fps = FPS_UNLIMITED) noexcept
        : target_ups (std::max(ups, MIN_UPS))
        , target_fps (fps)
    {
        reset();
    }

public:
    void setFPS(unsigned fps) noexcept {
        target_fps = fps;
        reset();
    }

    tick_info tick() noexcept {
        using namespace std::chrono;

        auto now = clock_type::now();

        updateTickWindow(now);

        last_now = curr_now;
        curr_now = now;

        //float interp = 0.f;
        unsigned update_count = (unsigned)std::min(size_t{ 10 }, fixed_tick - fixed_tick_prev);
        if (fixed_timescale > 0 && target_ups > 0) {
            interpolation = duration_cast<duration<float>>(curr_now - (fixed_start_offset + fixed_start)) / getUpsDuration();
        }

        tickCount += update_count;

        sec_update_counter += update_count;
        sec_frame_counter++;

        sec_accum += curr_now - last_now;
        while (sec_accum >= 1s) {
            info.avgFps = sec_frame_counter;
            info.avgUps = sec_update_counter;
            sec_frame_counter = 0;
            sec_update_counter = 0;
            sec_accum -= 1s;
        }

        float interp = ((target_ups == target_fps) && (fixed_timescale == 1.0)) ? 1.f : interpolation;
        float deltatime = static_cast<float>(sec_rep{curr_now - last_now}.count());

        return {
            update_count,
            interp,
            deltatime
        };
    }

    void sleep() noexcept {
        if (target_fps != FPS_UNLIMITED) {
            std::this_thread::sleep_until(frame_end);
        }
    }

    void reset() noexcept {
        using namespace std::chrono;
        fixed_tick = 0;
        frame_tick = 0;
        fixed_timescale_updated = true;
        updated_target_ups = true;

        sec_accum = 0s;
        sec_update_counter = 0;
        sec_frame_counter = 0;

        interpolation = 0.f;

        updateTickWindow(clock_type::now());
    }

    inline size_t getTickCount() const noexcept { return tickCount; }

public:
    void setTimescale(double timescale = 1.f) noexcept {
        fixed_timescale_updated |= (fixed_timescale != timescale);
        fixed_timescale = timescale;
    }

    inline void setTargetFPS(unsigned fps) noexcept { target_fps = fps; }
    inline void setTargetUPS(unsigned ups) noexcept {
        auto prev_ups = target_ups;
        target_ups = std::max(MIN_UPS, ups);
        updated_target_ups = prev_ups != target_ups;
    }

    inline unsigned getTargetFPS() const noexcept { return target_fps; }
    inline unsigned getTargetUPS() const noexcept { return target_ups; }

    inline unsigned getAvgFPS() const noexcept { return info.avgFps; }
    inline unsigned getAvgUPS() const noexcept { return info.avgUps; }

    seconds upsDuration() const noexcept {
        using namespace std::chrono;
        return sec_rep{time_res{1s} / target_ups}.count();
    }
    seconds fpsDuration() const noexcept {
        using namespace std::chrono;
        return sec_rep{time_res{1s} / target_fps}.count();
    }

private:
    void updateTickWindow(const time_point& now) noexcept {
        using namespace std::chrono;

        if (fixed_timescale_updated || updated_target_ups) {
            fixed_timescale_updated = false;
            updated_target_ups = false;

            fixed_tick_prev = 0;
            fixed_tick = 0;

            auto ups_delta = getUpsDuration();
            fixed_start_offset = time_point{((now.time_since_epoch() / ups_delta) * ups_delta)};
            //fixed_start_offset -= duration_cast<steady_clock::duration>( duration_cast<sec_rep>(ups_delta) * (1.f - interpolation) );
        }

        fixed_tick_prev = fixed_tick;

        if (fixed_timescale > 0 && target_ups > 0) {
            auto ups_delta = getUpsDuration();
            fixed_tick = (now - fixed_start_offset) / ups_delta;

            fixed_start = fixed_tick * ups_delta;
            fixed_end = fixed_start + ups_delta;
        }

        if (target_fps != FPS_UNLIMITED) {
            auto fps_delta = time_res{1s} / target_fps;
            frame_tick_prev = frame_tick;
            frame_tick = (now.time_since_epoch() / fps_delta);
            frame_start = time_point{(now.time_since_epoch() / fps_delta) * fps_delta};
            frame_end = frame_start + fps_delta;
        } else {
            frame_tick_prev = frame_tick;
            frame_tick++;
            frame_start = now;
            frame_end = now;
        }
    }

    clock_type::duration getUpsDuration() const noexcept {
        // using namespace std::chrono;
        return std::chrono::duration_cast<typename clock_type::duration>(
            std::chrono::duration<double>{ 1.0 / (fixed_timescale * static_cast<double>(target_ups)) }
        );
    }

    //std::optional<unsigned> next_ups;
    unsigned target_ups;
    unsigned target_fps;

    bool updated_target_ups = false;

    clock_type engineClock;
    time_res tickDuration;

    size_t tickCount = 0;

    time_point clock_start;
    time_point last_now;
    time_point curr_now;

    // for UPS
    double fixed_timescale = 1.0;
    bool fixed_timescale_updated = true;
    time_point fixed_start_offset;
    clock_type::duration fixed_start;
    clock_type::duration fixed_end;
    size_t fixed_tick_prev;
    size_t fixed_tick;

    // for FPS
    time_point frame_start;
    time_point frame_end;
    size_t frame_tick_prev;
    size_t frame_tick;

    float interpolation = 0.f;

    time_res sec_accum;
    size_t sec_update_counter = 0;
    size_t sec_frame_counter = 0;

    struct info {
        unsigned avgFps = 0;
        unsigned avgUps = 0;
    };
    info info;
};

}