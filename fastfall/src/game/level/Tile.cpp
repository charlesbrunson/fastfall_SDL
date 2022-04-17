#include "fastfall/game/level/Tile.hpp"

//#include "fastfall/util/direction.hpp"
#include "fastfall/util/log.hpp"

//#include <map>
#include <unordered_map>
#include <array>
//#include <ranges>

namespace ff {

	// Tile Touch

	TileTouch::TileTouch(TileShape t_shape)
		: shape(t_shape)
	{
		calc_value(shape);
	}

	uint8_t TileTouch::get_edge(Cardinal dir) const
	{
		uint8_t ret = 0;
		switch (dir)
		{
		case Cardinal::N:
			ret = (value & Top);
			break;
		case Cardinal::E:
			ret = (value & Right) >> 2;
			break;
		case Cardinal::S:
			ret = (value & Bot) >> 4;
			break;
		case Cardinal::W:
			ret = (value & Left) >> 6;
			break;
		}
		return ret;
	}
	bool TileTouch::get_corner(Ordinal dir) const
	{
		bool has = false;
		switch (dir)
		{
		case Ordinal::NW:
			has = (value & (Top_Left | Left_Top)) == (Top_Left | Left_Top);
			break;
		case Ordinal::NE:
			has = (value & (Top_Right | Right_Top)) == (Top_Right | Right_Top);
			break;
		case Ordinal::SE:
			has = (value & (Right_Bot | Bot_Right)) == (Right_Bot | Bot_Right);
			break;
		case Ordinal::SW:
			has = (value & (Bot_Left | Left_Bot)) == (Bot_Left | Left_Bot);
			break;
		}
		return has;
	}

	void TileTouch::calc_value(TileShape shape)
	{
		value = 0;

		switch (shape.type)
		{
		case TileShape::Type::Empty:
			break;
		case TileShape::Type::Solid:
			value = (Top | Right | Bot | Left);
			break;
		case TileShape::Type::Half:
			value = (Right_Bot | Bot | Left_Bot);
			break;
		case TileShape::Type::HalfVert:
			value = (Top_Left | Bot_Left | Left);
			break;
		case TileShape::Type::Slope:
			value = (Right | Bot);
			break;
		case TileShape::Type::Shallow1:
			value = (Right_Bot | Bot);
			break;
		case TileShape::Type::Shallow2:
			value = (Right | Bot | Left_Bot);
			break;
		case TileShape::Type::Steep1:
			value = (Top_Right | Right | Bot);
			break;
		case TileShape::Type::Steep2:
			value = (Right | Bot_Right);
			break;
		case TileShape::Type::Oneway:
			break;
		case TileShape::Type::OnewayVert:
			break;
		case TileShape::Type::LevelBoundary:
			value = (Top);
			break;
		case TileShape::Type::LevelBoundary_Wall:
			value = (Right);
			break;
		}

		auto swap_bits = [](uint8_t value, uint8_t bit1, uint8_t bit2, uint8_t shift_count)
		{
			assert(bit1 < bit2);
			return (value & ~(bit1 | bit2)) | ((value & bit1) << shift_count) | ((value & bit2) >> shift_count);
		};

		if (shape.flip_h)
		{
			value = swap_bits(value, Top_Left, Top_Right, 1);
			value = swap_bits(value, Bot_Left, Bot_Right, 1);
			value = swap_bits(value, Right, Left, 4);
		}
		if (shape.flip_v)
		{
			value = swap_bits(value, Right_Top, Right_Bot, 1);
			value = swap_bits(value, Left_Top, Left_Bot, 1);
			value = swap_bits(value, Top, Bot, 4);
		}
	}

	// Tile State

	bool autotile_has_edge(
		Cardinal dir,
		const TileTouch& auto_shape,
		const TileTouch& to_shape)
		//const cardinal_array<TileTouch>& edges)
	{
		using namespace direction;
		return (auto_shape.get_edge(dir) > 0)
			&& (auto_shape.get_edge(dir) == to_shape.get_edge(opposite(dir)));
	}

	bool autotile_has_inner_corner(
		Ordinal dir,
		const TileTouch& auto_shape,
		const cardinal_array<TileTouch>& edges,
		const ordinal_array<TileTouch>& corners)
	{
		using namespace direction;
		auto [V, H] = split(dir);

		const TileTouch& h_adj = edges[H];
		const TileTouch& v_adj = edges[V];
		const TileTouch& d_adj = corners[dir];

		bool corner_open = !autotile_has_edge(V, h_adj, d_adj)
						|| !autotile_has_edge(H, v_adj, d_adj);

		return autotile_has_edge(V, auto_shape, v_adj)
			&& autotile_has_edge(H, auto_shape, h_adj)
			&& auto_shape.get_corner(dir)
			&& corner_open;
	}


	TileState get_autotile_state(
		TileShape init_shape,
		grid_view<TileShape> grid,
		Vec2u position,
		std::variant<TileShape, AUTOTILE_GRID_WRAP> offgrid_shape)
	{
		auto dir_transform = [&]<typename T>(const std::initializer_list<T>& directions_list)
		{
			directional_array<TileTouch, T> out;
			for (auto dir : directions_list) {
				Vec2i vdir = direction::to_vector<int>(dir);
				Vec2u pos = position + vdir;

				if (grid.valid(pos)) {
					out[dir] = TileTouch{ grid[pos] };
				}
				else {
					if (std::holds_alternative<TileShape>(offgrid_shape))
					{
						out[dir] = TileTouch{ std::get<TileShape>(offgrid_shape) };
					}
					else
					{
						Vec2u wrapped_pos = position;
						if (vdir.x < 0 && wrapped_pos.x == 0) wrapped_pos.x += grid.column_count();
						if (vdir.y < 0 && wrapped_pos.y == 0) wrapped_pos.y += grid.row_count();
						wrapped_pos += vdir;
						wrapped_pos.x %= grid.column_count();
						wrapped_pos.y %= grid.row_count();

						out[dir] = TileTouch{ grid[wrapped_pos] };
					}
				}
			}
			return out;
		};

		TileTouch auto_shape{ init_shape };
		cardinal_array<TileTouch> neighbors_card = dir_transform(direction::cardinals);
		ordinal_array<TileTouch>  neighbors_ord  = dir_transform(direction::ordinals);

		TileState state;
		state.shape = init_shape;

		for ( auto dir : direction::cardinals ) {
			state.occupied_edges[dir] = autotile_has_edge(dir, auto_shape, neighbors_card[dir]);
			state.card_shapes[dir] = neighbors_card[dir].shape;
		}
		for ( auto dir : direction::ordinals ) {
			state.inner_corners[dir] = autotile_has_inner_corner(dir, auto_shape, neighbors_card, neighbors_ord);
			state.ord_shapes[dir] = neighbors_ord[dir].shape;
		}

		return state;
	}

	// Tile Constraint

	struct Match
	{
		enum class Priority
		{
			None = 0,
			Edge_Corner,
			Shape
		};

		Priority priority = Priority::None;
		unsigned match_count = 0;
		TileID tile_id;

		friend bool operator< (const Match& lhs, const Match& rhs)
		{
			if (lhs.priority != rhs.priority)
			{
				return lhs.priority < rhs.priority;
			}
			else if (lhs.match_count != rhs.match_count)
			{
				return lhs.match_count < rhs.match_count;
			}
			else return false;
		}

		friend bool operator> (const Match& lhs, const Match& rhs)
		{
			if (lhs.priority != rhs.priority)
			{
				return lhs.priority > rhs.priority;
			}
			else if (lhs.match_count != rhs.match_count)
			{
				return lhs.match_count > rhs.match_count;
			}
			else return false;
		}

		friend bool operator== (const Match& lhs, const Match& rhs)
		{
			return lhs.priority == rhs.priority
				&& lhs.match_count == rhs.match_count;
		}

		friend bool operator!= (const Match& lhs, const Match& rhs)
		{
			return lhs.priority != rhs.priority
				|| lhs.match_count != rhs.match_count;
		}
	};

	std::optional<Match> try_match(
		const TileState& state,
		const TileConstraint& constraint)
	{
		Match match;
		match.tile_id = constraint.tile_id;

		if (state.shape != constraint.shape)
			return std::nullopt;

		unsigned counter = 0;

		// check edges
		bool tested_edge_corner = false;

		for (auto dir : direction::cardinals)
		{
			if (auto opt_edge = constraint.edges[dir];
				opt_edge && std::holds_alternative<bool>(*opt_edge))
			{
				tested_edge_corner = true;
				bool check = (std::get<bool>(*opt_edge) == state.occupied_edges[dir]);
				if (check) {
					counter++;
				}
				else {
					//match.match_count = 0;
					return {};
				}
			}
		}
		for (auto dir : direction::ordinals)
		{
			if (auto opt_corner = constraint.corners[dir]; opt_corner)
			{
				tested_edge_corner = true;
				bool check = (*opt_corner == state.inner_corners[dir]);
				if (check) {
					counter++;
				}
				else {
					//match.match_count = 0;
					return {};
				}
			}
		}

		if (tested_edge_corner)
		{
			match.priority = Match::Priority::Edge_Corner;
			match.match_count = counter;
		}

		// check shapes
		counter = 0;

		bool tested_shape = false;

		for (auto dir : direction::cardinals)
		{
			if (auto edge = constraint.edges[dir];
				edge && std::holds_alternative<TileShape>(*edge))
			{
				tested_shape = true;
				bool check = (std::get<TileShape>(*edge) == state.card_shapes[dir]);
				if (check) {
					counter++;
				}
				else {
					//match.match_count = 0;
					return {};
				}
			}
		}
		if (tested_shape)
		{
			match.priority = Match::Priority::Shape;
			match.match_count = counter;
		}

		return match;
	}

	std::optional<TileID> auto_best_tile(
		const TileState& state,
		const std::vector<TileConstraint>& constraints,
		unsigned seed)
	{
		//std::vector<Match> matches;
		std::vector<Match> matches;
		matches.reserve(constraints.size());

		for (const auto& constraint : constraints)
		{
			if (auto match = try_match(state, constraint); match.has_value()) {
				matches.insert(
					std::upper_bound(matches.begin(), matches.end(), *match), 
					*match);
			}
		}

		// pick best
		if (!matches.empty())
		{
			auto& first = *matches.rbegin();
			unsigned count = 0;
			for (auto rit = matches.rbegin(); rit != matches.rend(); rit++)
			{
				if (*rit == first)
					count++;
				else
					break;
			}

			// multiple valid matches, pick random one
			srand(seed);
			TileID tID = (matches.rbegin() + (rand() % count))->tile_id;
			srand(1);

			return tID;
		}
		else
		{
			return std::nullopt;
		}
	}

	// Tile Materials

	namespace {
		std::unordered_map<std::string, TileMaterial> materials;
	}

	const TileMaterial Tile::standardMat = {
		.typeName = "standard"
	};

	void Tile::addMaterial(const TileMaterial& mat) {
		materials.insert_or_assign(mat.typeName, mat);
	}

	const TileMaterial& Tile::getMaterial(std::string typeName) {
		const auto it = materials.find(typeName);

		if (it != materials.end()) {
			return (it->second);
		}
		else {
			return standardMat;
		}
	}

}
