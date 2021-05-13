#pragma once

typedef double secs;

constexpr secs ms_to_secs(unsigned ms) noexcept {
	return 0.001 * ms;
}

