#pragma once

#include "fastfall/engine/time/time.hpp"

#include <chrono>
using namespace std::chrono;

using time_res = nanoseconds;
using sec_rep = duration<double>;
//static_assert(treat_as_floating_point<sec_rep::rep>::value, "sec_rep required to be floating point");

class EngineClock {
public:
	EngineClock(unsigned int fps = 60, bool steady = true);

public:
	secs tick() noexcept;

	// optionally sleeps whole thread until next tick when called
	// implicitly calls resetTickWindow()
	void sleepUntilTick(bool nosleep = false) noexcept;

	void reset() noexcept;

public:
	void setTargetFPS(unsigned int fps = 60) noexcept;
	unsigned getTargetFPS() const noexcept;
	unsigned getAvgFPS() const noexcept;
	unsigned getInstantFPS() const noexcept;

public:
	inline secs getElapsed() { return sec_rep(engineClock.now() - tick_start_p).count(); }
	inline secs getTickDuration() const noexcept { return sec_rep( tickDuration ).count(); }
	inline unsigned getTickCount() const noexcept { return _data.tickTotal; }
	inline time_point<steady_clock> getTickStart() { return tick_start_p; }
	inline time_point<steady_clock> getTickEnd() { return tick_end_p; }

public:
	// useful to know data
	struct FrameData {
		// duration of sleepUntilTick()
		sec_rep sleepTime = 0s;

		// duration between tick() and sleepUntilTick()
		sec_rep activeTime = 0s;

		sec_rep lastElapsed = 0s;

		unsigned avgFps = 0u;

		unsigned tickTotal = 0u;

		unsigned tickMissPerSec = 0u;
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
	steady_clock engineClock;

	// current tick window
	time_point<steady_clock> tick_start_p;
	time_point<steady_clock> tick_end_p;

	// elapsed time points
	time_point<steady_clock> last_elapsed_p;
	time_point<steady_clock> cur_elapsed_p;

	// Tick
	long long cur_tick;
	long long prev_tick;
	
	// FPS tracking
	//Framerate fpsEnum;
	bool fpsUnlimited;
	unsigned targetFps;
	unsigned tickMissCounter;
	unsigned tickCounter;
	time_res elapsedAcc;

	//----------------
	FrameData _data;

	time_point<steady_clock> _active_start;
	time_point<steady_clock> _sleep_start;
};
