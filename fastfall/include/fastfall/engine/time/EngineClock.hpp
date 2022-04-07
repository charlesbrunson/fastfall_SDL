#pragma once

#include "fastfall/engine/time/time.hpp"

#include <thread>
#include <chrono>


class EngineClock {
public:
	using clock_type = std::conditional<
		std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock >::type;


	using time_point = std::chrono::time_point<clock_type>;
	using time_res = std::chrono::nanoseconds;
	using sec_rep = std::chrono::duration<double>;

public:
	EngineClock(unsigned int fps, bool steady = true);

public:
	secs tick() noexcept;

	// optionally sleeps the thread until next tick when called
	// implicitly calls resetTickWindow()
	void sleepUntilTick(bool nosleep = false) noexcept;

	void reset() noexcept;

	// FPS management
	void setTargetFPS(unsigned int fps) noexcept;
	unsigned getTargetFPS() const noexcept;
	unsigned getAvgFPS() const noexcept;
	unsigned getInstantFPS() const noexcept;

	void(*sleep_until_fn)(const time_point) = [](const time_point sleep_time)
	{
		std::this_thread::sleep_until(sleep_time);
	};

public:
	inline secs getElapsed() { return sec_rep(engineClock.now() - tick_start_p).count(); }
	inline secs getTickDuration() const noexcept { return sec_rep( tickDuration ).count(); }
	inline unsigned getTickCount() const noexcept { return _data.tickTotal; }
	inline time_point getTickStart() { return tick_start_p; }
	inline time_point getTickEnd() { return tick_end_p; }

	inline unsigned missTicks() const noexcept { return tickMissCounter; }

public:
	// useful to know data
	struct FrameData {
		sec_rep sleepTime = sec_rep::zero();	// sleep duration of last sleepUntilTick() call
		sec_rep activeTime = sec_rep::zero();	// elapsed time between tick() and sleepUntilTick()
		unsigned avgFps = 0u;					// number of times tick() was called in the last second, updates every second
		unsigned tickTotal = 0u;				// total times tick() has been called
		unsigned tickMissPerSec = 0u;			// number of ticks missed per second
	};
	inline const FrameData& data() { return _data; }
	inline void setSteady(bool steadytick) { steadyTick = steadytick; };

private:
	// steadyTick makes tick() return the average elapse time for our target FPS
	// provided the clock has not missed a tick
	bool steadyTick = false;

	void resetTickWindow(bool nomiss = false) noexcept;

	// target tick duration for our FPS
	time_res tickDuration;

	// time since previous tick()
	time_res elapsed;

	// the big clock
	clock_type engineClock;

	// current tick window
	time_point tick_start_p;
	time_point tick_end_p;

	// elapsed time points
	time_point last_elapsed_p;
	time_point cur_elapsed_p;

	// Tick
	size_t cur_tick;
	size_t prev_tick;
	
	// FPS tracking
	bool fpsUnlimited;
	unsigned targetFps;
	unsigned tickMissCounter;
	unsigned tickCounter;
	time_res elapsedAcc;

	//----------------
	FrameData _data;

	time_point _active_start;
	time_point _sleep_start;
};
