#include "fastfall/engine/time/EngineClock.hpp"

#include <thread>
#include <iostream>
#include <thread>
#include <chrono>

#include "fastfall/util/log.hpp"

//#include <chrono>
using namespace std::chrono;

EngineClock::EngineClock(unsigned int fps, bool steady):
	steadyTick(steady)
{
	elapsed = 0ns;
	elapsedAcc = 0ns;

	tickCounter = 0;
	tickMissCounter = 0;
	cur_tick = 0;

	setTargetFPS(fps);
}

void EngineClock::setTargetFPS(unsigned int fps) noexcept {

	targetFps = fps;
	fpsUnlimited = (targetFps == 0);

	if (!fpsUnlimited) {
		tickDuration = time_res{ 1s } / targetFps;
	}
	else {
		_data.sleepTime = 0s;
		tickDuration = 0s;
	}

	resetTickWindow(true);
}
unsigned EngineClock::getTargetFPS() const noexcept {
	return targetFps;
}
unsigned EngineClock::getAvgFPS() const noexcept {
	return _data.avgFps;
}
unsigned EngineClock::getInstantFPS() const noexcept {
	return (elapsed > 0s ? (time_res{ 1s } / elapsed) : 0);
}

void EngineClock::sleepUntilTick(bool nosleep) noexcept {

	secs PREV_TIME = _data.activeTime.count();

	_data.activeTime = engineClock.now() - _active_start;

	secs TIME = _data.activeTime.count();
	if (fpsUnlimited)
		return;

	if (engineClock.now() < tick_end_p && !nosleep) {
		_sleep_start = engineClock.now();
		std::this_thread::sleep_until(tick_end_p);
		_data.sleepTime = engineClock.now() - _sleep_start;
	}
	else {
		_data.sleepTime = 0s;
	}
	resetTickWindow();
}

void EngineClock::resetTickWindow(bool nomiss) noexcept {
	if (fpsUnlimited)
		return;

	time_point<steady_clock> now = engineClock.now();

	if (tickDuration > 0s) {
		prev_tick = cur_tick;
		cur_tick = now.time_since_epoch() / tickDuration;
		if (prev_tick == cur_tick) {
			cur_tick++;
		}
	}

	tick_start_p = time_point<steady_clock>( (cur_tick * tickDuration));
	tick_end_p = tick_start_p + tickDuration;

	if (tickDuration > 0s) {
		if (!nomiss && prev_tick != 0 && prev_tick + 1 != cur_tick) {
			tickMissCounter += cur_tick - prev_tick - 1;
		}
	}
}

secs EngineClock::tick() noexcept {
	_active_start = engineClock.now();
	if (prev_tick != 0) {
		tickCounter++;
		_data.tickTotal++;
	}

	cur_elapsed_p = engineClock.now();

	elapsed = cur_elapsed_p - last_elapsed_p;
	last_elapsed_p = cur_elapsed_p;


	// update avg fps
	elapsedAcc += elapsed;
	while (elapsedAcc >= 1s) {
		_data.avgFps = tickCounter;
		_data.tickMissPerSec = tickMissCounter;
		tickCounter = 0;
		tickMissCounter = 0;
		elapsedAcc -= 1s;
	}

	if (!fpsUnlimited && steadyTick && prev_tick + 1 == cur_tick) {
		return sec_rep( tickDuration ).count();
	}
	return sec_rep( elapsed ).count();
}

void EngineClock::reset() noexcept {

	elapsed = 0ns;
	elapsedAcc = 0ns;

	last_elapsed_p = tick_start_p;
	tickCounter = 0;
	tickMissCounter = 0;
	_data.tickTotal = 0;
	cur_tick = 0;
	resetTickWindow();
}
