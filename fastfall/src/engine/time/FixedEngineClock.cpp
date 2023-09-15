#include "fastfall/engine/time/FixedEngineClock.hpp"

#include <algorithm>
#include <thread>

FixedEngineClock::FixedEngineClock(unsigned ups, unsigned fps) noexcept
	: target_ups (std::max(ups, MIN_UPS))
	, target_fps (fps)
{
	reset();
}

void FixedEngineClock::setFPS(unsigned fps) noexcept
{
	target_fps = fps;
	reset();
}




FixedEngineClock::clock_type::duration FixedEngineClock::getUpsDuration() const noexcept {
    using namespace std::chrono;
    return duration_cast<clock_type::duration>( duration<double>{ 1.0 / (fixed_timescale * static_cast<double>(target_ups)) } );
}

FixedEngineClock::Tick FixedEngineClock::tick() noexcept
{
	using namespace std::chrono;

    auto now = clock_type::now();

	updateTickWindow(now);

	last_now = curr_now;
	curr_now = now;

    float interp = 0.f;
	unsigned update_count = (unsigned)std::min(size_t{ 10 }, fixed_tick - fixed_tick_prev);
    if (fixed_timescale > 0 && target_ups > 0) {
        interp = duration_cast<duration<float>>(curr_now - (fixed_start_offset + fixed_start)) / getUpsDuration();
    }

	tickCount += update_count;

	sec_update_counter += update_count;
	sec_frame_counter++;

	sec_accum += curr_now - last_now;
	while (sec_accum >= 1s) {
		stats.avgFps = sec_frame_counter;
		stats.avgUps = sec_update_counter;
		sec_frame_counter = 0;
		sec_update_counter = 0;
		sec_accum -= 1s;
	}

	return {
		sec_rep{ curr_now - last_now }.count(),
		update_count,
		((target_ups == target_fps) && (fixed_timescale == 1.0)) ? 1.f : interp
	};
}
void FixedEngineClock::sleep() noexcept 
{
	if (target_fps != FPS_UNLIMITED)
	{
		std::this_thread::sleep_until(frame_end);
	}
}

secs FixedEngineClock::upsDuration() const noexcept {
	using namespace std::chrono;
	return sec_rep{ time_res{ 1s } / target_ups }.count();
}
secs FixedEngineClock::fpsDuration() const noexcept {
	using namespace std::chrono;
	return sec_rep{ time_res{ 1s } / target_fps }.count();
}


void FixedEngineClock::reset() noexcept
{
	using namespace std::chrono;

	fixed_tick = 0;
	frame_tick = 0;
    fixed_timescale_updated = true;
    updated_target_ups      = true;

	sec_accum          = 0s;
	sec_update_counter = 0;
	sec_frame_counter  = 0;

	updateTickWindow(clock_type::now());
}

void FixedEngineClock::setTimescale(double timescale) noexcept {
    fixed_timescale_updated |= (fixed_timescale != timescale);
    fixed_timescale = timescale;
};

void FixedEngineClock::updateTickWindow(const clock_type::time_point& now) noexcept
{
	using namespace std::chrono;


    if (fixed_timescale_updated || updated_target_ups) {
        fixed_timescale_updated = false;
        updated_target_ups      = false;

        fixed_tick_prev = 0;
        fixed_tick      = 0;

        auto ups_delta = getUpsDuration();
        fixed_start_offset = time_point{ ( now.time_since_epoch() / ups_delta) * ups_delta };
    }

    fixed_tick_prev = fixed_tick;

    if (fixed_timescale > 0 && target_ups > 0) {
        auto ups_delta = getUpsDuration();
        fixed_tick = (now - fixed_start_offset) / ups_delta;

        fixed_start = fixed_tick  * ups_delta;
        fixed_end   = fixed_start + ups_delta;
    }


	if (target_fps != FPS_UNLIMITED) {
		auto fps_delta  = time_res{ 1s } / target_fps;
		frame_tick_prev = frame_tick;
		frame_tick      = (now.time_since_epoch() / fps_delta);
		frame_start     = time_point{ (now.time_since_epoch() / fps_delta) * fps_delta };
		frame_end       = frame_start + fps_delta;
	}
	else {
		frame_tick_prev = frame_tick;
		frame_tick++;
		frame_start =  now;
		frame_end   =  now;
	}
}
