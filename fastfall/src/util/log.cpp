#include "fastfall/util/log.hpp"

#include <iostream>
#include <algorithm>
#include <mutex>

//namespace ff {

std::deque<log::LogMessage> log::messages;
log::level log::logVerbosity = log::level::NONE;
unsigned log::currentDepth = 0u;
unsigned log::currentTick = 0u;

log::scope::scope() {
	if ((added = (currentDepth < maxDepth))) {
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


void log::detail_log(level lvl, const std::string_view& file, uint32_t line, std::string& msg) noexcept {

	static constexpr char fill[] = {
		' ',
		' ',
		' ',
		' ',
		'-',
		'!'
	};
	static constexpr std::string_view lvlStr[] = {
		"NONE",
		"STEP",
		"VERB",
		"INFO",
		"WARN",
		"ERR",
	};

	static std::mutex mut;
	std::scoped_lock lock(mut);

	unsigned lvlNdx = static_cast<unsigned>(lvl);

	if (messages.size() == LOG_HISTORY_SIZE) { messages.pop_front(); }

	std::string_view levelStr = lvlStr[lvlNdx];

	std::string fileContent = fmt::format("{}:{}", file, line);
	if (fileContent.size() > 30)
		fileContent.resize(30, ' ');

	msg.insert(msg.begin(), (size_t)currentDepth * 2, ' ');
	if (fileContent.size() > 80)
		fileContent.resize(80, ' ');

	messages.push_back(LogMessage{
		.lvl = lvl,
		.message = fmt::format("{:<10}{:<5s}{:<30s}{:<s}", currentTick, levelStr, fileContent, msg)
	});

	int fillSize = 28 - fileContent.size();
	if (fillSize > 0) {
		char fillChar = fill[lvlNdx];
		std::fill_n(&messages.back().message[15 + fileContent.size() + 1], fillSize, fillChar);
	}

	if (lvl >= get_verbosity())
		fmt::print("{}\n", messages.back().message);
}




//}