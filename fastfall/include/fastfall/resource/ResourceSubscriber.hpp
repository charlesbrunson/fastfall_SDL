#pragma once

#include "Resources.hpp"

#include <unordered_set>

namespace ff {

class ResourceSubscriber {
public:
	ResourceSubscriber();
    ResourceSubscriber(const ResourceSubscriber&);
    ResourceSubscriber(ResourceSubscriber&&) noexcept;
    ResourceSubscriber& operator=(const ResourceSubscriber&);
    ResourceSubscriber& operator=(ResourceSubscriber&&) noexcept;
	virtual ~ResourceSubscriber();

	virtual void notifyReloadedAsset(const Asset* asset) = 0;

	void subscribe(const Asset* asset);
	void unsubscribe(const Asset* asset);

	bool is_subscribed(const Asset* asset) {
		return asset_subs.contains(asset);
	}

    void unsubscribe_all();

	static std::unordered_set<ResourceSubscriber*>& getAll() { return subscribers; };

private:
	std::unordered_set<const Asset*> asset_subs;
	static std::unordered_set<ResourceSubscriber*> subscribers;
};

}