#pragma once

#include "fastfall/engine/time/time.hpp"

#include <algorithm>
#include <chrono>

class FixedEngineClock {
private:
	using clock_type = std::conditional<
		std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock >::type;

	using time_point = std::chrono::time_point<clock_type>;
	using time_res   = std::chrono::nanoseconds;
	using sec_rep    = std::chrono::duration<double>;

public:
	constexpr static unsigned MIN_UPS = 10;
	constexpr static unsigned FPS_UNLIMITED = 0;

	constexpr static size_t UPDATE_MAX = 3;

	struct Tick {
		secs elapsed;
		unsigned update_count;
		float interp_value;
	};

public:
	FixedEngineClock(unsigned ups = 60, unsigned fps = FPS_UNLIMITED) noexcept;

public:
	void setFPS(unsigned fps) noexcept;
	Tick tick() noexcept;
	void sleep() noexcept;

	void reset() noexcept;

	inline size_t getTickCount() const noexcept { return tickCount; }

public:
    void setTimescale(double timescale = 1.f) noexcept;

	inline void setTargetFPS(unsigned fps)  noexcept { target_fps = fps; }
	inline void setTargetUPS(unsigned ups)  noexcept {
        auto prev_ups = target_ups;
        target_ups    = std::max(MIN_UPS, ups);
        updated_target_ups = prev_ups != target_ups;
    }

	inline unsigned getTargetFPS()  const noexcept { return target_fps; }
	inline unsigned getTargetUPS()  const noexcept { return target_ups; }

	inline unsigned getAvgFPS()		const noexcept { return stats.avgFps; }
	inline unsigned getAvgUPS()		const noexcept { return stats.avgUps; }

	secs upsDuration() const noexcept;
	secs fpsDuration() const noexcept;

private:
	void updateTickWindow(const clock_type::time_point& now) noexcept;

    clock_type::duration getUpsDuration() const noexcept;

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
    double     fixed_timescale = 1.0;
    bool       fixed_timescale_updated = true;
    clock_type::time_point fixed_start_offset;
    clock_type::duration   fixed_start;
    clock_type::duration   fixed_end;
	size_t     fixed_tick_prev;
	size_t     fixed_tick;

	// for FPS
	time_point frame_start;
	time_point frame_end;
	size_t frame_tick_prev;
	size_t frame_tick;

	time_res sec_accum;
	size_t sec_update_counter = 0;
	size_t sec_frame_counter = 0;

	struct Stats {
		unsigned avgFps = 0;
		unsigned avgUps = 0;

	};
	Stats stats;

};
