#pragma once

#include <deque>
#include <iostream>
#include <sstream>
#include <chrono>
#include <utility>

//#include <source_location>

#include "fmt/core.h"

#if defined(_WIN32)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#elif defined(unix)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined(__APPLE__)
// TODO
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
		ERR = 5,
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
	template<class... Args>
	static void _log(level lvl, const std::string_view& file, int line, const std::string_view& format, Args&&...args) noexcept {
		if (lvl < get_verbosity())
			return;

		static constexpr char fill[] = {
			' ',
			' ',
			' ',
			' ',
			'-',
			'!'
		};

		unsigned lvlNdx = static_cast<unsigned>(lvl);

		if (messages.size() == LOG_HISTORY_SIZE) { messages.pop_front(); }

		std::string_view levelStr = lvlStr[lvlNdx];

		std::string fileContent = fmt::format("{}:{}", file, line);
		if (fileContent.size() > 30)
			fileContent.resize(30, ' ');

		std::string msgContent = fmt::format(format, std::forward<Args>(args)...);

		msgContent.insert(msgContent.begin(), (size_t)currentDepth * 2, ' ');
		if (fileContent.size() > 80)
			fileContent.resize(80, ' ');

		messages.push_back(LogMessage{
			.lvl = lvl,
			.message = fmt::format("{:<10}{:<5s}{:<30s}{:<s}", currentTick, levelStr, fileContent, msgContent)
			});

		int fillSize = 28 - fileContent.size();
		if (fillSize > 0) {
			char fillChar = fill[lvlNdx];
			std::fill_n(&messages.back().message[15 + fileContent.size() + 1], fillSize, fillChar);
		}


		if (lvl >= get_verbosity())
			std::cout << messages.back().message << std::endl;
	}

	static inline void set_tick(unsigned tick) { currentTick = tick; }

private:
	static unsigned currentTick;

	static log::level logVerbosity;
	static unsigned currentDepth;
	static constexpr unsigned maxDepth = 10u;

	static constexpr unsigned LOG_HISTORY_SIZE = 50;
	static std::deque<log::LogMessage> messages;

	static constexpr std::string_view lvlStr[] = {
		"NONE",
		"STEP",
		"VERB",
		"INFO",
		"WARN",
		"ERR",
	};

};

//}

#ifdef DEBUG
	#define LOG_STEP( ... ) log::_log( log::level::STEP, __FILENAME__, __LINE__, __VA_ARGS__ );
	#define LOG_VERB( ... ) log::_log( log::level::VERB, __FILENAME__, __LINE__, __VA_ARGS__ );
	#define LOG_INFO( ... ) log::_log( log::level::INFO, __FILENAME__, __LINE__, __VA_ARGS__ );
	#define LOG_WARN( ... ) log::_log( log::level::WARN, __FILENAME__, __LINE__, __VA_ARGS__ );
	#define LOG_ERR_( ... ) log::_log( log::level::ERR,  __FILENAME__, __LINE__, __VA_ARGS__ );
#else
	#define LOG_NONE( message )
	#define LOG_STEP( message )
	#define LOG_VERB( message )
	#define LOG_INFO( message )
	#define LOG_WARN( message )
	#define LOG_ERR_( message )
#endif

