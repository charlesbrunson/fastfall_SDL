#pragma once

#include <deque>
#include <iostream>
#include <sstream>
#include <chrono>
#include <utility>
#include <string_view>
#include <filesystem>

//#include <source_location>

//#include "fmt/core.h"
#include "fmt/format.h"

#include <source_location>

#if not defined(__FILENAME__)
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))

//	#if defined(_WIN32)
//	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
//	#else
//	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
//	#endif
#endif


//namespace ff {

class log {
public:

	struct scope {
		scope();
		~scope();

		scope(const scope&) = delete;
		scope(scope&&) = delete;

		scope& operator= (const scope&) = delete;
		scope& operator= (scope&&) = delete;

	private:
		bool added = false;
	};

	enum class level : unsigned char {
		NONE = 0,
		STEP = 1,
		VERB = 2,
		INFO = 3,
		WARN = 4,
		ERR  = 5,
		SIZE = 6
	};

	struct LogMessage {
		level lvl = level::NONE;
		std::string message;
	};

	static void set_verbosity(level level) noexcept;
	static level get_verbosity() noexcept;
	static const std::deque<log::LogMessage>& get_entries() noexcept {
		return messages;
	}

	// log internal call, use appropriate LOG_ macros instead
	template<typename...T>
	static void _log(const level lvl, const std::source_location loc, fmt::format_string<T...> log_fmt, T&&... log_args) noexcept {
		detail_log(lvl, loc, log_fmt.get(), fmt::make_format_args(log_args...));
	}

	static void set_tick(unsigned tick) { currentTick = tick; }

private:
	static void detail_log(level lvl, std::source_location loc, fmt::string_view fmt, fmt::format_args args) noexcept;

	static unsigned currentTick;

	static log::level logVerbosity;
	static unsigned currentDepth;
	static constexpr unsigned maxDepth = 10u;

	static constexpr unsigned LOG_HISTORY_SIZE = 1000;
	static std::deque<log::LogMessage> messages;
};

#ifdef DEBUG
	#define LOG_STEP( ... ) log::_log(log::level::STEP, std::source_location::current(), __VA_ARGS__)
	#define LOG_VERB( ... ) log::_log(log::level::VERB, std::source_location::current(), __VA_ARGS__)
	#define LOG_INFO( ... ) log::_log(log::level::INFO, std::source_location::current(), __VA_ARGS__)
	#define LOG_WARN( ... ) log::_log(log::level::WARN, std::source_location::current(), __VA_ARGS__)
	#define LOG_ERR_( ... ) log::_log(log::level::ERR,  std::source_location::current(), __VA_ARGS__)
#else
	#define LOG_NONE( ... )
	#define LOG_STEP( ... )
	#define LOG_VERB( ... )
	#define LOG_INFO( ... )
	#define LOG_WARN( ... )
	#define LOG_ERR_( ... )
#endif

