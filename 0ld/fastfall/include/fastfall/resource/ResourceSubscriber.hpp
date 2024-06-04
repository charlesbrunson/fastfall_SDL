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

	void subscribe_asset(const Asset* asset);
	void unsubscribe_asset(const Asset* asset);

	bool is_subscribed_to_asset(const Asset* asset) {
		return asset_subs.contains(asset);
	}

    void unsubscribe_all_assets();

	static std::unordered_set<ResourceSubscriber*>& get_asset_subscriptions() { return subscribers; };

private:
	std::unordered_set<const Asset*> asset_subs;
	static std::unordered_set<ResourceSubscriber*> subscribers;
};

}