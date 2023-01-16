#pragma once

#include "fastfall/resource/Asset.hpp"

#include <filesystem>
#include <atomic>
#include <mutex>
#include <thread>

namespace ff {

class ResourceWatcher {
public:
	// adds resource to list to watch
	static void add_watch(Asset* asset, const std::vector<std::filesystem::path>& file );

	// removes resource from list to watch
	static void remove_watch(Asset* asset);

	// clears list of watched resources
	static void clear_watch();

	// starts the watch thread
	static void start_watch_thread();

	// signals watch thread to stop
	static void stop_watch_thread();

	// waits for watch thread to stop
	static void join_watch_thread();

	static bool is_watch_running() { return is_watching; };

private:

	struct File {
		File() {};
		File(std::filesystem::path t_file);

		std::filesystem::path path;
		std::filesystem::file_time_type last_modified;
	};

	struct Watchable {
		Asset* asset;
		std::vector<File> files;
	};

	static std::optional<std::filesystem::file_time_type> is_file_modified(File& file);

	static std::mutex watchable_mut;
	static std::vector<Watchable> watchables;

	static void routine_watch();
	static std::thread watcher;

	static std::atomic_bool to_watch;
	static std::atomic_bool is_watching;

};

}