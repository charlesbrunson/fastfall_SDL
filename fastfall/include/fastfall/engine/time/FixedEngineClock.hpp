#pragma once

#include "fastfall/engine/time/time.hpp"

#include <chrono>

class FixedEngineClock {
private:
	using clock_type = std::conditional<
		std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock >::type;

	using time_point = std::chrono::time_point<clock_type>;
	using time_res = std::chrono::nanoseconds;
	using sec_rep = std::chrono::duration<double>;

public:
	constexpr static unsigned MIN_UPS = 60;
	constexpr static unsigned FPS_UNLIMITED = 0;

	constexpr static size_t UPDATE_MAX = 3;

	struct Tick {
		secs elapsed;
		unsigned update_count;
		float interp_value;
	};

public:
	FixedEngineClock(unsigned ups = MIN_UPS, unsigned fps = FPS_UNLIMITED) noexcept;

public:
	void setFPS(unsigned fps) noexcept;
	Tick tick() noexcept;
	void sleep() noexcept;

	void reset() noexcept;
public:
	unsigned getTargetFPS()  const noexcept;
	unsigned getAvgFPS()	 const noexcept;
	unsigned getInstantFPS() const noexcept;

	secs upsDuration() const noexcept;
	secs fpsDuration() const noexcept;

private:
	void updateTickWindow() noexcept;

	unsigned target_ups;
	unsigned target_fps;

	clock_type engineClock;
	time_res tickDuration;

	time_point clock_start;
	time_point last_now;
	time_point curr_now;

	// for UPS
	time_point fixed_start;
	time_point fixed_end;
	size_t fixed_tick_prev;
	size_t fixed_tick;

	// for FPS
	time_point frame_start;
	time_point frame_end;
	size_t frame_tick_prev;
	size_t frame_tick;
};