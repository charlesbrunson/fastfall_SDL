#pragma once

#include "fastfall/game/level/TileID.hpp"
#include "fastfall/game/level/TileShape.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/util/direction.hpp"
#include "fastfall/util/grid_vector.hpp"

#include <variant>

namespace ff {

	// Tile Touch

	struct TileTouch
	{
		TileTouch() = default;
		TileTouch(TileShape t_shape);

		uint8_t get_edge(Cardinal dir) const;
		bool get_corner(Ordinal dir) const;

		TileShape shape = {};

	private:
		uint8_t value = 0;

		enum Edge : uint16_t {
			Top_Left = 1 << 0,
			Top_Right = 1 << 1,
			Right_Top = 1 << 2,
			Right_Bot = 1 << 3,
			Bot_Left = 1 << 4,
			Bot_Right = 1 << 5,
			Left_Top = 1 << 6,
			Left_Bot = 1 << 7,

			Top = Top_Left | Top_Right,
			Right = Right_Top | Right_Bot,
			Bot = Bot_Left | Bot_Right,
			Left = Left_Top | Left_Bot,
		};

		void calc_value(TileShape shape);
	};

	// Tile State

	struct TileState
	{
		TileShape shape;
		cardinal_array<bool> occupied_edges = { false };
		ordinal_array<bool>  inner_corners = { false };

		cardinal_array<TileShape> card_shapes = {};
		ordinal_array<TileShape>  ord_shapes = {};
	};

	struct AUTOTILE_GRID_WRAP {};

	TileState get_autotile_state(
		TileShape init_shape,
		grid_view<TileShape> grid,
		Vec2u position,
		std::variant<TileShape, AUTOTILE_GRID_WRAP> offgrid_shape
	);

	// Tile Constraint

	namespace tile_constraint_options {
		inline constexpr std::nullopt_t n_a{ std::nullopt };
		inline constexpr bool			no{ false };
		inline constexpr bool			yes{ true };
	}

	struct TileConstraint {
		using Edge = std::optional<std::variant<bool, TileShape>>;
		using Corner = std::optional<bool>;

		TileID tile_id;
		TileShape shape;
		cardinal_array<Edge> edges = {};
		ordinal_array<Corner> corners = {};
	};

	std::optional<TileID> auto_best_tile(
		const TileState& state,
		const std::vector<TileConstraint>& constraints,
		unsigned seed = time(0));

	// Tile Material

	struct SurfaceMaterial {
		float velocity = 0.f;
	};

	struct TileMaterial {
		const SurfaceMaterial& getSurface(Cardinal side, Cardinal facing = Cardinal::N) const {
			size_t ndx = (static_cast<int>(side) - static_cast<int>(facing)) % 4;
			return surfaces.at(ndx);
		}

		std::string typeName;
		std::array<SurfaceMaterial, 4> surfaces;
	};

	class TilesetAsset;

	// Tile

	class Tile {
	public:
		TileID id;
		const TilesetAsset* origin = nullptr;

		TileShape shape;
		Cardinal matFacing = Cardinal::N;

		// tile next reference
		TileID next_offset = TileID{};
		std::optional<unsigned> next_tileset = std::nullopt;

		bool auto_substitute = false;

		bool has_next_tileset() const {
			return next_tileset.has_value();
		}

		static const TileMaterial standardMat;
		static void addMaterial(const TileMaterial& mat);
		static const TileMaterial& getMaterial(std::string typeName);
	};

}