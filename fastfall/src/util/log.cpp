#include "fastfall/util/log.hpp"

#include <iostream>
#include <algorithm>

//namespace ff {

std::deque<log::LogMessage> log::messages;
log::level log::logVerbosity = log::level::NONE;
unsigned log::currentDepth = 0u;
unsigned log::currentTick = 0u;

log::scope::scope() {
	if (added = (currentDepth < maxDepth)) {
		currentDepth++;
	}
}
log::scope::~scope() {
	if (added && currentDepth > 0u) {
		currentDepth--;
	}
}

void log::set_verbosity(level level) noexcept {
	logVerbosity = level;
}
log::level log::get_verbosity() noexcept {
	return logVerbosity;
}

//}