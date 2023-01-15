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

ResourceWatcher::File::File(std::filesystem::path t_file) 
	: path(t_file)
	, last_modified(check_modified_time(t_file))
{
}


void ResourceWatcher::add_watch(Asset* asset, const std::vector<std::filesystem::path>& files) {
	std::scoped_lock<std::mutex> lock(watchable_mut);

	watchables.push_back(
		Watchable{
			.asset = asset
		}
	);


	std::transform(
		files.begin(),
		files.end(),
		std::back_inserter(watchables.back().files),
		[asset](const std::filesystem::path& path) -> File {
			namespace fs = std::filesystem;

			fs::path root = path.parent_path();
			while (root.stem().generic_string() != "data" || root == path.root_directory()) {
				root = root.parent_path();
			}

			std::string relative_pathstr = path.generic_string().substr(root.generic_string().length() + 1);

			LOG_INFO("{:40} \"{}\"", asset->get_name(), relative_pathstr);
			return File{ path };
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

std::pair<bool, std::filesystem::file_time_type> ResourceWatcher::is_file_modified(File& file) {

	std::filesystem::file_time_type mod_time = check_modified_time(file.path);

	return std::make_pair(mod_time > file.last_modified, mod_time);
}

void ResourceWatcher::routine_watch() {
	using namespace std::chrono_literals;
	is_watching = true;

	std::unordered_set<Asset*> to_update;

	while (to_watch) {
		to_update.clear();

		for (auto& watchable : watchables) {

			bool is_modified = false;

			for (auto& file : watchable.files) 
			{
				if (auto [mod, time] = is_file_modified(file); mod)
				{
					file.last_modified = time;
					is_modified = true;
				}
			}
			if (is_modified) {
				to_update.insert(watchable.asset);
				LOG_INFO("Asset \"{}\" is outdated", watchable.asset->get_name());
			}
		}
		for (Asset* asset : to_update) {
			asset->setOutOfDate(true);
		}
		std::this_thread::sleep_for(1s);
	}

	is_watching = false;
}

}