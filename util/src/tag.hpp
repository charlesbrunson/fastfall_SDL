#pragma once

#include <string_view>
#include <unordered_map>
#include <sstream>
#include <concepts>
#include <algorithm>
#include <type_traits>
#include <numeric>

#include "fastfall/util/log.hpp"

namespace ff {

template<typename T>
concept is_tag = requires (T t, std::string_view tag_name) {
	tag_name = T::TagType;
};

template<typename T>
requires is_tag<T>
struct Tag {
private:

	unsigned short tag_id = 0;
	static constexpr unsigned short kLimit = std::numeric_limits<unsigned short>::max() - 1;

	std::string_view str;

	std::unordered_map<std::string, size_t>& getTags() {
		static std::unordered_map<std::string, size_t> tags;
		return tags;
	}

public:

	constexpr Tag() = default;
	constexpr Tag(const char* tag) : Tag(std::string(tag)) {};
	Tag(std::string tag)
	{
		if (tag.empty())
			return;

		std::for_each(tag.begin(), tag.end(), [](char c) -> char { return std::tolower(c); } );

		auto it = getTags().find(tag);

		if (it == getTags().end()) {
			if (getTags().size() < kLimit) {
				it = getTags().insert(std::make_pair(tag, static_cast<unsigned short>(getTags().size()) + 1)).first;
			}
			else {
				LOG_ERR_("cannot create tag \"{}\", tag type {} at max size of {}", tag, T::TagType, kLimit);
			}
		}
		if (it != getTags().end()) {
			tag_id = it->second;
			str = it->first;
		}
	}

	constexpr std::string_view tag_type_str() const { return T::TagType; }
	constexpr std::string_view tag_name_str() const { return str; }
	std::string to_string() const {
		std::stringstream ss;
		ss << T::TagType << ":" << (str.empty() ? "NIL" : str);
		return ss.str();
	}

	constexpr unsigned short id() const { return tag_id; }
	constexpr bool empty() const { return tag_id == 0; }

	friend constexpr bool operator< (const Tag<T>& lhs, const Tag<T>& rhs) {
		return lhs.tag_id < rhs.tag_id;
	}
	friend constexpr bool operator== (const Tag<T>& lhs, const Tag<T>& rhs) {
		return lhs.tag_id == rhs.tag_id;
	}
};


struct actor_group_tag_traits { constexpr static const char* TagType = "object_group"; };
struct trigger_tag_traits { constexpr static const char* TagType = "trigger"; };

using ActorGroupTag = Tag<actor_group_tag_traits>;
using TriggerTag = Tag<trigger_tag_traits>;

}


template<typename T>
struct std::hash<ff::Tag<T>>
{
	size_t operator()(const ff::Tag<T>& x) const
	{
		return x.id();
	}
};
