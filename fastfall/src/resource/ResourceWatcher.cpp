#include "fastfall/resource/ResourceWatcher.hpp"

#include <unordered_set>

#include "fastfall/util/log.hpp"

namespace ff {

std::mutex ResourceWatcher::watchable_mut;
std::vector<ResourceWatcher::Watchable> ResourceWatcher::watchables;

std::atomic_bool ResourceWatcher::to_watch = false;
std::atomic_bool ResourceWatcher::is_watching = false;

std::thread ResourceWatcher::watcher;

std::filesystem::file_time_type check_modified_time(const std::filesystem::path& file) {
	return std::filesystem::last_write_time(file);
}

void ResourceWatcher::add_watch(const std::filesystem::path& file, Asset* asset) {
	std::scoped_lock<std::mutex> lock(watchable_mut);

	watchables.push_back(
		Watchable{
			.file = file,
			.last_modified = check_modified_time(file),
			.asset = asset
		}
	);
}

void ResourceWatcher::remove_watch(Asset* asset) {
	std::scoped_lock<std::mutex> lock(watchable_mut);

	watchables.erase(
		std::remove_if(watchables.begin(), watchables.end(),
			[&asset](const Watchable& watch) {
				return (watch.asset == asset);
			}
		),
		watchables.end());

}

void ResourceWatcher::clear_watch() {
	std::scoped_lock<std::mutex> lock(watchable_mut);
	watchables.clear();
}

void ResourceWatcher::start_watch_thread() {
	assert(!is_watching && !to_watch);

	to_watch = true;
	watcher = std::thread(ResourceWatcher::routine_watch);
}

void ResourceWatcher::stop_watch_thread() {
	assert(is_watching && to_watch);
	to_watch = false;
}

void ResourceWatcher::join_watch_thread() {
	watcher.join();
}

std::pair<bool, std::filesystem::file_time_type> ResourceWatcher::is_file_modified(Watchable& watch) {

	std::filesystem::file_time_type mod_time = check_modified_time(watch.file);

	return std::make_pair(mod_time > watch.last_modified, mod_time);
}

void ResourceWatcher::routine_watch() {
	using namespace std::chrono_literals;
	is_watching = true;

	std::unordered_set<Asset*> to_update;

	while (to_watch) {
		to_update.clear();

		for (auto& watchable : watchables) {
			if (auto [mod, time] = is_file_modified(watchable); mod) {
				watchable.last_modified = time;

				LOG_INFO("\"{}\" has been updated", watchable.file.string());

				//watchable.asset->reloadFromFile();
				to_update.insert(watchable.asset);
			}
		}
		for (Asset* asset : to_update) {
			asset->reloadFromFile();
		}
		std::this_thread::sleep_for(1s);
	}

	is_watching = false;
}

}