#pragma once

#include "ff/core/time.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

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

    clock(unsigned ups = 60, unsigned fps = FPS_UNLIMITED) noexcept
        : m_target_ups (std::max(ups, MIN_UPS))
        , m_target_fps (fps)
    {
        reset();
    }

    void setFPS(unsigned fps) noexcept {
        m_target_fps = fps;
        reset();
    }

    tick_info tick() noexcept {
        using namespace std::chrono;

        auto now = clock_type::now();

        updateTickWindow(now);

        m_last_now = m_curr_now;
        m_curr_now = now;

        //float interp = 0.f;
        unsigned update_count = (unsigned)std::min(size_t{ 10 }, m_fixed_tick - m_fixed_tick_prev);
        if (m_fixed_timescale > 0 && m_target_ups > 0) {
            m_interpolation = duration_cast<duration<float>>(m_curr_now - (m_fixed_start_offset + m_fixed_start)) / getUpsDuration();
        }

        m_tick_count += update_count;

        m_sec_update_counter += update_count;
        m_sec_frame_counter++;

        m_sec_accum += m_curr_now - m_last_now;
        while (m_sec_accum >= 1s) {
            m_info.avg_fps = m_sec_frame_counter;
            m_info.avg_ups = m_sec_update_counter;
            m_sec_frame_counter = 0;
            m_sec_update_counter = 0;
            m_sec_accum -= 1s;
        }

        float interp = ((m_target_ups == m_target_fps) && (m_fixed_timescale == 1.0)) ? 1.f : m_interpolation;
        float deltatime = static_cast<float>(sec_rep{m_curr_now - m_last_now}.count());

        return {
            update_count,
            m_interpolate ? interp : 1.f,
            deltatime
        };
    }

    void sleep() noexcept {
        if (m_target_fps != FPS_UNLIMITED) {
            std::this_thread::sleep_until(m_frame_end);
        }
    }

    void reset() noexcept {
        using namespace std::chrono;
        m_fixed_tick = 0;
        m_frame_tick = 0;
        m_fixed_timescale_updated = true;
        m_updated_target_ups = true;

        m_sec_accum = 0s;
        m_sec_update_counter = 0;
        m_sec_frame_counter = 0;

        m_interpolation = 0.f;

        updateTickWindow(clock_type::now());
    }

    inline size_t getTickCount() const noexcept { return m_tick_count; }

    void setTimescale(double timescale = 1.f) noexcept {
        m_fixed_timescale_updated |= (m_fixed_timescale != timescale);
        m_fixed_timescale = timescale;
    }

    inline void setTargetFPS(unsigned fps) noexcept { m_target_fps = fps; }
    inline void setTargetUPS(unsigned ups) noexcept {
        auto prev_ups = m_target_ups;
        m_target_ups = std::max(MIN_UPS, ups);
        m_updated_target_ups = prev_ups != m_target_ups;
    }

    inline unsigned getTargetFPS() const noexcept { return m_target_fps; }
    inline unsigned getTargetUPS() const noexcept { return m_target_ups; }

    inline unsigned getAvgFPS() const noexcept { return m_info.avg_fps; }
    inline unsigned getAvgUPS() const noexcept { return m_info.avg_ups; }

    seconds upsDuration() const noexcept {
        using namespace std::chrono;
        return sec_rep{time_res{1s} / m_target_ups}.count();
    }
    seconds fpsDuration() const noexcept {
        using namespace std::chrono;
        return sec_rep{time_res{1s} / m_target_fps}.count();
    }

    bool m_interpolate  = true;
    bool m_pause_update = false;
    bool m_step_update  = false;

private:
    void updateTickWindow(const time_point& now) noexcept {
        using namespace std::chrono;

        if (m_fixed_timescale_updated || m_updated_target_ups) {
            m_fixed_timescale_updated = false;
            m_updated_target_ups = false;

            m_fixed_tick_prev = 0;
            m_fixed_tick = 0;

            auto ups_delta = getUpsDuration();
            m_fixed_start_offset = time_point{((now.time_since_epoch() / ups_delta) * ups_delta)};
            //fixed_start_offset -= duration_cast<steady_clock::duration>( duration_cast<sec_rep>(ups_delta) * (1.f - interpolation) );
        }

        m_fixed_tick_prev = m_fixed_tick;

        if (m_fixed_timescale > 0 && m_target_ups > 0) {
            auto ups_delta = getUpsDuration();
            m_fixed_tick = (now - m_fixed_start_offset) / ups_delta;

            m_fixed_start = m_fixed_tick * ups_delta;
            m_fixed_end = m_fixed_start + ups_delta;
        }

        if (m_target_fps != FPS_UNLIMITED) {
            auto fps_delta = time_res{1s} / m_target_fps;
            m_frame_tick_prev = m_frame_tick;
            m_frame_tick = (now.time_since_epoch() / fps_delta);
            m_frame_start = time_point{(now.time_since_epoch() / fps_delta) * fps_delta};
            m_frame_end = m_frame_start + fps_delta;
        } else {
            m_frame_tick_prev = m_frame_tick;
            m_frame_tick++;
            m_frame_start = now;
            m_frame_end = now;
        }
    }

    clock_type::duration getUpsDuration() const noexcept {
        // using namespace std::chrono;
        return std::chrono::duration_cast<typename clock_type::duration>(
            std::chrono::duration<double>{ 1.0 / (m_fixed_timescale * static_cast<double>(m_target_ups)) }
        );
    }

    //std::optional<unsigned> next_ups;
    unsigned m_target_ups;
    unsigned m_target_fps;

    bool m_updated_target_ups = false;

    clock_type m_engine_clock;
    time_res m_tick_duration;

    size_t m_tick_count = 0;

    time_point m_clock_start;
    time_point m_last_now;
    time_point m_curr_now;

    // for UPS
    double m_fixed_timescale = 1.0;
    bool m_fixed_timescale_updated = true;
    time_point m_fixed_start_offset;
    clock_type::duration m_fixed_start;
    clock_type::duration m_fixed_end;
    size_t m_fixed_tick_prev;
    size_t m_fixed_tick;

    // for FPS
    time_point m_frame_start;
    time_point m_frame_end;
    size_t m_frame_tick_prev;
    size_t m_frame_tick;

    float m_interpolation = 0.f;

    time_res m_sec_accum;
    size_t m_sec_update_counter = 0;
    size_t m_sec_frame_counter = 0;

    struct info {
        unsigned avg_fps = 0;
        unsigned avg_ups = 0;
    };
    info m_info;
};

}