#pragma once

#include <string>
#include <array>

namespace ff {

	class TileShape {
	public:
		enum class Type : uint8_t {
			Empty,
			Solid,
			Half,
			HalfVert,
			Slope,
			Shallow1,
			Shallow2,
			Steep1,
			Steep2,
			Oneway,
			OnewayVert,

			LevelBoundary,
			LevelBoundary_Wall,
		};

		constexpr static inline unsigned TypeCount = 13U;

		Type type = Type::Empty;
		bool flip_h = false;
		bool flip_v = false;

		constexpr static TileShape from_string(std::string_view str);
		std::string to_string() const {
			std::string str{ get_type_label(type) };
			if (flip_h || flip_v)
			{
				str += '-';
				if (flip_h) str += 'h';
				if (flip_v) str += 'v';
			}
			return str;
		}

		constexpr bool operator== (const TileShape& rhs) const;
		constexpr bool operator!= (const TileShape& rhs) const;

		constexpr static bool has_hori_symmetry(TileShape::Type type);
		constexpr static bool has_vert_symmetry(TileShape::Type type);

		constexpr static std::string_view get_type_label(TileShape::Type type);

	private:
		constexpr static std::array type_labels = {
			"empty",
			"solid",
			"half",
			"halfvert",
			"slope",
			"shallow1",
			"shallow2",
			"steep1",
			"steep2",
			"oneway",
			"onewayvert"
		};
	};

	constexpr TileShape operator""_ts(const char* c_str, size_t len)
	{
		return TileShape::from_string(std::string_view{ c_str, len });
	}

	constexpr TileShape TileShape::from_string(std::string_view str)
	{
		std::string_view shape_str = str;
		std::string_view flip_str = "";

		if (auto ndx = str.find_first_of('-');
			ndx < str.size())
		{
			shape_str = str.substr(0, ndx);
			flip_str = str.substr(ndx + 1);
		}

		Type shape_type = Type::Empty;
		for (int i = 0; i < TileShape::type_labels.size(); i++)
		{
			if (TileShape::type_labels[i] == shape_str)
			{
				shape_type = static_cast<Type>(i);
				break;
			}
		}

		return TileShape{
			.type = shape_type,
			.flip_h = !has_hori_symmetry(shape_type)
					  && (flip_str.find_first_of('h') != std::string_view::npos),
			.flip_v = !has_vert_symmetry(shape_type)
					  && (flip_str.find_first_of('v') != std::string_view::npos)
		};
	}

//	std::string TileShape::to_string() const
//	{
//		std::string str{ get_type_label(type) };
//		if (flip_h || flip_v)
//		{
//			str += '-';
//			if (flip_h) str += 'h';
//			if (flip_v) str += 'v';
//		}
//		return str;
//	}

	constexpr bool TileShape::operator== (const TileShape& rhs) const
	{
		return type == rhs.type
			&& flip_h == rhs.flip_h
			&& flip_v == rhs.flip_v;
	}

	constexpr bool TileShape::operator!= (const TileShape& rhs) const
	{
		return !(*this == rhs);
	}

	constexpr bool TileShape::has_hori_symmetry(TileShape::Type type)
	{
		constexpr bool h_sym[] = {
			true,  // Empty,
			true,  // Solid,
			true,  // Half,
			false, // HalfVert,
			false, // Slope,
			false, // Shallow1,
			false, // Shallow2,
			false, // Steep1,
			false, // Steep2,
			true,  // Oneway,
			false, // OnewayVert
		};
		return h_sym[static_cast<unsigned>(type)];
	}

	constexpr bool TileShape::has_vert_symmetry(TileShape::Type type)
	{
		constexpr bool h_sym[] = {
			true,  // Empty,
			true,  // Solid,
			false, // Half,
			true,  // HalfVert,
			false, // Slope,
			false, // Shallow1,
			false, // Shallow2,
			false, // Steep1,
			false, // Steep2,
			false, // Oneway,
			true,  // OnewayVert
		};
		return h_sym[static_cast<unsigned>(type)];
	}


	constexpr std::string_view TileShape::get_type_label(TileShape::Type type)
	{
		return TileShape::type_labels[static_cast<unsigned>(type)];
	}

}
