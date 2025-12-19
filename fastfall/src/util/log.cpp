#include "fastfall/util/log.hpp"

#include <iostream>
#include <algorithm>
#include <mutex>

#include "fmt/color.h"

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


void log::detail_log(level lvl, const std::source_location loc, fmt::string_view fmt, fmt::format_args args) noexcept {

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

	auto file = std::filesystem::path{loc.file_name()}.filename().string();
	std::string fileContent = fmt::format("{}:{}", file, loc.line());

	if (fileContent.size() > 30)
		fileContent.resize(30, ' ');

	if (fileContent.size() > 80)
		fileContent.resize(80, ' ');

	messages.push_back(LogMessage{
		.lvl = lvl,
		.message = fmt::format("{:<10}{:<5s}{:<30s}{:\t>{}}{:<s}", currentTick, levelStr, fileContent, "\t", currentDepth, fmt::vformat(fmt, args))
	});

	int fillSize = 28 - fileContent.size();
	if (fillSize > 0) {
		char fillChar = fill[lvlNdx];
		std::fill_n(&messages.back().message[15 + fileContent.size() + 1], fillSize, fillChar);
	}

	fmt::color color = fmt::color::white;
	switch (lvl) {
		case level::NONE: color = fmt::color::black; break;
		case level::STEP: color = fmt::color::dark_gray; break;
		case level::VERB: color = fmt::color::gray; break;
		case level::INFO: color = fmt::color::light_gray; break;
		case level::WARN: color = fmt::color::yellow; break;
		case level::ERR : color = fmt::color::red; break;
		default: ;
	}

	if (lvl >= get_verbosity())
		fmt::print(fmt::fg(color), "{}\n", messages.back().message);
}




//}