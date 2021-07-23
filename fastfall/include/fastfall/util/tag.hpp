

#include <string_view>
#include <unordered_map>
#include <sstream>
#include <concepts>
#include <algorithm>

namespace ff {

template<typename T>
requires requires (T t) {
	{ T::TagType } -> std::convertible_to<std::string_view>;
}
struct Tag {
private:

	unsigned tag_id = 0;
	std::string_view str;

	std::unordered_map<std::string, unsigned>& getTags() {
		static std::unordered_map<std::string, unsigned> tags;
		return tags;
	}

public:

	constexpr Tag() = default;
	constexpr Tag(const char* tag) : Tag(std::string(tag)) {};
	constexpr Tag(std::string tag)
	{
		if (tag.empty())
			return;

		std::for_each(tag.begin(), tag.end(), [](char c) -> char { return std::tolower(c); } );

		auto it = getTags().find(tag);

		if (it == getTags().end()) {
			it = getTags().insert(std::make_pair(tag, getTags().size() + 1)).first;
		}
		tag_id = it->second;
		str = it->first;
	}

	constexpr std::string_view tag_type_str() const { return T::TagType; }
	constexpr std::string_view tag_name_str() const { return str; }
	constexpr std::string to_string() const {
		std::stringstream ss;
		ss << T::TagType << ":" << (str.empty() ? "NIL" : str);
		return ss.str();
	}

	constexpr unsigned id() const { return tag_id; }
	constexpr bool empty() const { return tag_id == 0; }

	friend constexpr bool operator< (const Tag<T>& lhs, const Tag<T>& rhs) {
		return lhs.tag_id < rhs.tag_id;
	}
	friend constexpr bool operator== (const Tag<T>& lhs, const Tag<T>& rhs) {
		return lhs.tag_id == rhs.tag_id;
	}
};


struct object_tag_traits { constexpr static const char* TagType = "object"; };
struct trigger_tag_traits { constexpr static const char* TagType = "trigger"; };

using ObjectTag = Tag<object_tag_traits>;
using TriggerTag = Tag<trigger_tag_traits>;

}


namespace std
{
	template<typename T>
	struct hash<ff::Tag<T>>
	{
		size_t operator()(const ff::Tag<T>& x) const
		{
			return x.id();
		}
	};
}