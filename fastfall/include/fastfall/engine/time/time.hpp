#pragma once

#include <cmath>

typedef double secs;

constexpr secs ms_to_secs(size_t ms) noexcept {
	return 0.001 * ms;
}

