#pragma once

#include "fastfall/resource/Asset.hpp"

#include <filesystem>
#include <atomic>
#include <mutex>

namespace ff {

class ResourceWatcher {
public:
	static void add_watch(const std::filesystem::path& file, Asset* asset);

	static void remove_watch(Asset* asset);

	static void clear_watch();

	static void start_watch_thread();

	static void stop_watch_thread();

	static void join_watch_thread();


private:

	struct Watchable {
		std::filesystem::path file;
		std::filesystem::file_time_type last_modified;

		//std::string asset_name;
		Asset* asset;
	};

	static std::pair<bool, std::filesystem::file_time_type> is_file_modified(Watchable& watch);

	static std::mutex watchable_mut;
	static std::vector<Watchable> watchables;

	static void routine_watch();
	static std::thread watcher;

	static std::atomic_bool to_watch;
	static std::atomic_bool is_watching;

};

}