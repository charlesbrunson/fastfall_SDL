#include "fastfall/resource/ResourceSubscriber.hpp"

namespace ff {

std::unordered_set<ResourceSubscriber*> ResourceSubscriber::subscribers;

ResourceSubscriber::ResourceSubscriber() {
	subscribers.insert(this);
}
ResourceSubscriber::~ResourceSubscriber() {
	subscribers.erase(this);
}

void ResourceSubscriber::subscribe(const Asset* asset) {
	asset_subs.insert(asset);
}

void ResourceSubscriber::unsubscribe(const Asset* asset) {
	asset_subs.erase(asset);
}

}