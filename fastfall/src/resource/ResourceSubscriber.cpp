#include "fastfall/resource/ResourceSubscriber.hpp"

namespace ff {

std::unordered_set<ResourceSubscriber*> ResourceSubscriber::subscribers;

ResourceSubscriber::ResourceSubscriber() {
	subscribers.insert(this);
}

ResourceSubscriber::ResourceSubscriber(const ResourceSubscriber& rhs) {
    subscribers.insert(this);
    asset_subs = rhs.asset_subs;
}

ResourceSubscriber::ResourceSubscriber(ResourceSubscriber&& rhs) noexcept {
    subscribers.insert(this);
    asset_subs = std::move(rhs.asset_subs);
}

ResourceSubscriber& ResourceSubscriber::operator=(const ResourceSubscriber& rhs) {
    subscribers.insert(this);
    asset_subs = rhs.asset_subs;
}

ResourceSubscriber& ResourceSubscriber::operator=(ResourceSubscriber&& rhs) noexcept {
    subscribers.insert(this);
    asset_subs = std::move(rhs.asset_subs);
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
void ResourceSubscriber::unsubscribe_all() {
    asset_subs.clear();
}

}