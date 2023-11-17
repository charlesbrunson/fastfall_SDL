#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/util/PlotLine.hpp"

#include <algorithm>
#include <stdlib.h>
#include <cmath>

#include "fastfall/render/DebugDraw.hpp"

namespace ff {


	ColliderTileMap::ColliderTileMap(Vec2i size, bool border) :
		ColliderRegion(),
		hasBorder(border),
		size_min(0, 0),
		size_max(size)
	{
		// adjust for borders
		if (hasBorder) {
			size_min += Vec2i(-1, -1);
			size_max += Vec2i(1, 1);
		}

		collisionMapSize = Vec2u{ size_max - size_min };

		minIndex = 0u;
		maxIndex = ((size_t)collisionMapSize.y * collisionMapSize.x);

		clear();

		//debug_dirtyFlag = true;
		boundingBox = Rectf(
			Vec2f(size_min * TILESIZE_F),
			Vec2f(size_max * TILESIZE_F)
		);
		prevBoundingBox = boundingBox;
	}

	void ColliderTileMap::update(secs deltaTime) {
		applyChanges();

		if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDER)) {
			bool is_redrawn = debugDrawQuad(validCollisionSize, &tileCollisionMap[0], getPosition(), this, update_debugDraw);
			if (update_debugDraw)
				update_debugDraw = !is_redrawn;
		}
	}

	const ColliderQuad* ColliderTileMap::get_quad(QuadID quad_id) const noexcept
	{
		return tileShapeMap[quad_id.value].hasTile ? &tileCollisionMap[quad_id.value] : nullptr;
	}

    Vec2i ColliderTileMap::to_pos(QuadID quad_id) const noexcept {
        Vec2i position;
        position.y = quad_id.value / (size_max.x - size_min.x);
        position.x = quad_id.value - (position.y * (size_max.x - size_min.x));
        position += size_min;
        return position;
    }

	bool ColliderTileMap::on_precontact(World& w, const ContinuousContact& contact, secs duration) const {
        auto quad_id = contact.id->quad;
		if (validPosition(quad_id) && callback_on_precontact) {
			return callback_on_precontact(w, contact, duration);
		}
		return true;
	}

	void ColliderTileMap::on_postcontact(World& w, const AppliedContact& contact, secs deltaTime) const {
		if (validPosition(contact.id->quad) && callback_on_postcontact) {
			callback_on_postcontact(w, contact, deltaTime);
		}
	}

	void ColliderTileMap::setBorders(const Vec2u& size, const unsigned cardinalBits) {
		if (!hasBorder)	return;

		using namespace direction;

		for (auto side : cardinals) {
			if (cardinalBits & to_bits(side)) {

				TileShape shape;
				//shape.shapeTouches = to_bits(opposite(side));

				shape.type =
					((side == Cardinal::N) || (side == Cardinal::S) ?
						TileShape::Type::LevelBoundary :
						TileShape::Type::LevelBoundary_Wall);

				shape.flip_v = (side == Cardinal::N);
				shape.flip_h = (side == Cardinal::E);

				switch (side) {
				case Cardinal::N: for (unsigned x = 0; x < size.x; x++) setTile(Vec2i(x, -1), shape); break;
				case Cardinal::E:  for (unsigned y = 0; y < size.y; y++) setTile(Vec2i(size.x, y), shape); break;
				case Cardinal::S: for (unsigned x = 0; x < size.x; x++) setTile(Vec2i(x, size.y), shape); break;
				case Cardinal::W:  for (unsigned y = 0; y < size.y; y++) setTile(Vec2i(-1, y), shape); break;
				}
			}
			else {
				switch (side) {
				case Cardinal::N: for (unsigned x = 0; x < size.x; x++) removeTile(Vec2i(x, -1)); break;
				case Cardinal::E:  for (unsigned y = 0; y < size.y; y++) removeTile(Vec2i(size.x, y)); break;
				case Cardinal::S: for (unsigned x = 0; x < size.x; x++) removeTile(Vec2i(x, size.y)); break;
				case Cardinal::W:  for (unsigned y = 0; y < size.y; y++) removeTile(Vec2i(-1, y)); break;
				}
			}
		}
	}



	void ColliderTileMap::clear() {
		tileCollisionMap = grid_vector<ColliderQuad>(collisionMapSize.x, collisionMapSize.y);
		tileShapeMap = grid_vector<TileTable>(collisionMapSize.x, collisionMapSize.y);

		int i = 0;
		for (auto& col : tileCollisionMap) {
			col.setID({ i++ });
		}
		validCollisionSize = 0;
	}

	void ColliderTileMap::applyChanges() {
		if (editQueue.empty())
			return;

		//Vec2i size = size_max - size_min;
		std::vector<bool> impacted(maxIndex - minIndex);

		Vec2i change_min{ INT_MAX, INT_MAX };
		Vec2i change_max{ -INT_MAX, -INT_MAX };

		bool any_change = false;

		while (!editQueue.empty()) {
			bool has_change = false;
			const Edit& change = editQueue.front();
			const Vec2i& pos = change.position;

			if (change.removal) {
				has_change = applyRemoveTile(change);
			}
			else {
				has_change = applySetTile(change);
			}

			if (has_change) 
			{
				change_min.x = std::min(std::max(pos.x - 1, size_min.x), change_min.x);
				change_min.y = std::min(std::max(pos.y - 1, size_min.y), change_min.y);

				change_max.x = std::max(std::min(pos.x + 1, size_max.x - 1), change_max.x);
				change_max.y = std::max(std::min(pos.y + 1, size_max.y - 1), change_max.y);

				any_change = true;

				for (int xx = pos.x - 1; xx <= pos.x + 1; xx++) {
					for (int yy = pos.y - 1; yy <= pos.y + 1; yy++) 
					{
						Vec2i pos{ xx,yy };
						if (validPosition(pos))
						{
							impacted[getTileID(pos).value] = true;
						}
					}
				}
			}
			editQueue.pop();
		}

		if (any_change) {
			for (int yy = change_min.y; yy <= change_max.y; yy++) {
				for (int xx = change_min.x; xx <= change_max.x; xx++) {
					Vec2i pos{ xx, yy };
					if (impacted[getTileID(pos).value]) {
						updateGhosts(pos);
					}
				}
			}
		}
	}

	bool ColliderTileMap::applyRemoveTile(const Edit& change) {

		auto [quad, tile] = get_tile(change.position);

		if (!quad)
			return false;

		for_adjacent_touching_quads(*tile,
			[&](ColliderQuad& quad_adj, const ColliderTile& tile_adj, Cardinal dir)
			{
				ColliderQuad original = tile_adj.toQuad(quad_adj.getID());
				Cardinal other_dir = direction::opposite(dir);

				const ColliderSurface* originalSurf = original.getSurface(other_dir);
				if (originalSurf) {
					if (!quad_adj.hasAnySurface()) {
						incr_valid_collision();
					}

					quad_adj.setSurface(other_dir, *originalSurf);
				}
			});

		if (quad->hasAnySurface()) {
			quad->clearSurfaces();
			decr_valid_collision();
		}

		tileShapeMap[getTileID(change.position).value].hasTile = false;
		tileShapeMap[getTileID(change.position).value].tile.shape = TileShape{};
		return true;
	}

	bool isTileGeometryDifferent(const ColliderTile& lhs, const ColliderTile& rhs)
	{
		return lhs.shape.type != rhs.shape.type
			|| lhs.shape.flip_h != rhs.shape.flip_h
			|| lhs.shape.flip_v != rhs.shape.flip_v;;
	}

	bool isTileMaterialDifferent(const ColliderTile& lhs, const ColliderTile& rhs)
	{
		return (lhs.mat != rhs.mat || lhs.matFacing != rhs.matFacing);
	}

	//returns indicate of which surfaces to be erased (corresponds to params)
	std::pair<bool, bool> cullTouchingSurfaces(ColliderSurface& lhs, ColliderSurface& rhs) {

		auto r = std::make_pair(false, false);

		if (lhs.surface.p1 == rhs.surface.p2 &&
			lhs.surface.p2 == rhs.surface.p1) {

			r.first = true;
			r.second = true;
		}
		else if (lhs.surface.p1 == rhs.surface.p2 || lhs.surface.p2 == rhs.surface.p1) {
			Vec2f diff_added = lhs.surface.p1 - lhs.surface.p2;
			Vec2f diff_adjacent = rhs.surface.p1 - rhs.surface.p2;

			float len_added = abs(diff_added.x == 0 ? diff_added.y : diff_added.x);
			float len_adjacent = abs(diff_adjacent.x == 0 ? diff_adjacent.y : diff_adjacent.x);

			if (len_added + len_adjacent <= TILESIZE)
				return r;

			if (len_added > len_adjacent) {
				if (lhs.surface.p1 == rhs.surface.p2) {
					lhs.surface.p1 = rhs.surface.p1;
				}
				else {
					lhs.surface.p2 = rhs.surface.p2;
				}
				r.second = true;
			}
			else if (len_adjacent > len_added) {
				if (rhs.surface.p1 == lhs.surface.p2) {
					rhs.surface.p1 = lhs.surface.p1;
				}
				else {
					rhs.surface.p2 = lhs.surface.p2;
				}
				r.first = true;
			}
		}
		return r;
	}

	bool ColliderTileMap::applySetTile(const Edit& change) {

		QuadID ndx = getTileID(change.position);
		ColliderTile nTile(change.position, change.toShape, change.material, change.matFacing);
		ColliderQuad nQuad = nTile.toQuad( ndx );

		// tile exists at this position
		if (auto [quad, tile] = get_tile(change.position); quad) {

			if (isTileGeometryDifferent(*tile, nTile) 
				|| isTileMaterialDifferent(*tile, nTile))
			{
				applyRemoveTile(Edit{ change.position, true });
			}
			else {
				// no impact to collision map
				return false;
			}
		}

		if (nTile.shape.type == TileShape::Type::Empty) {
			tileShapeMap[ndx.value].hasTile = false;
			tileShapeMap[ndx.value].tile = nTile;
			tileCollisionMap[ndx.value] = nQuad;
			return true;
		}

		if (isTileGeometryDifferent(ColliderTile{}, nTile)) {
			for_adjacent_touching_quads(nTile,
				[&](ColliderQuad& quad_adj, const ColliderTile& tile_adj, Cardinal side)
				{
					ColliderSurface* added = nQuad.getSurface(side);
					ColliderSurface* adjacent = quad_adj.getSurface(direction::opposite(side));

					if (added == nullptr || adjacent == nullptr) {
						return;
					}

					auto [first, second] = cullTouchingSurfaces(*added, *adjacent);

					if (first) {
						nQuad.removeSurface(side);
					}
					if (second) {
						quad_adj.removeSurface(direction::opposite(side));

						if (!quad_adj.hasAnySurface()) {
							decr_valid_collision();
						}
					}
				});
		}

		if (nQuad.hasAnySurface()) {
			incr_valid_collision();
		}

		tileShapeMap[ndx.value].hasTile = true;
		tileShapeMap[ndx.value].tile = nTile;
		tileCollisionMap[ndx.value] = nQuad;
		return true;
	}


	bool ColliderTileMap::validPosition(const Vec2i& at) const noexcept {

		return at.x >= size_min.x && at.x < size_max.x
			&& at.y >= size_min.y && at.y < size_max.y;
	}
	bool ColliderTileMap::validPosition(QuadID ndx) const noexcept {
		return ndx.value >= minIndex && ndx.value < maxIndex;
	}

	QuadID ColliderTileMap::getTileID(const Vec2i& at) const noexcept {
        int value = ((size_t)at.x - size_min.x) + (((size_t)at.y - size_min.y) * collisionMapSize.x) ;
		return QuadID{ value };
	}

	std::pair<ColliderQuad*, const ColliderTile*> ColliderTileMap::get_tile(const Vec2i& at) {
		QuadID ndx = getTileID(at);
		if (validPosition(ndx)) {
			auto& it = tileShapeMap[ndx.value];
			return it.hasTile ? std::make_pair(&tileCollisionMap[ndx.value], &it.tile) : std::make_pair(nullptr, &tileShapeMap[ndx.value].tile);
		}
		return std::make_pair(nullptr, nullptr);
	};

	const ColliderQuad* ColliderTileMap::get_quad(const Vec2i& at) const noexcept {

		auto ndx = getTileID(at);
		if (validPosition(ndx))
		{
			return (tileShapeMap[ndx.value].hasTile ? &tileCollisionMap[ndx.value] : nullptr);
		}
		return nullptr;
	}

    Recti ColliderTileMap::get_tile_area_for_rect(Rectf area) const {
        Rectf bbox = math::shift(area, -getPosition());

        float left   = bbox.left;
        float top    = bbox.top;
        float right  = bbox.left + bbox.width;
        float bottom = bbox.top  + bbox.height;

        float left_r   = fmodf(left,   TILESIZE_F);
        float top_r    = fmodf(top,    TILESIZE_F);
        float right_r  = fmodf(right,  TILESIZE_F);
        float bottom_r = fmodf(bottom, TILESIZE_F);

        float epsilon = 0.01f;

        if (left_r < epsilon) left -= 16.f;
        if (top_r  < epsilon) top  -= 16.f;

        right  += 16.f;
        bottom += 16.f;

        if (right_r  > TILESIZE_F - epsilon) right  += 16.f;
        if (bottom_r > TILESIZE_F - epsilon) bottom += 16.f;

        int lefti   = static_cast<int>(left)   / (int)TILESIZE;
        int topi    = static_cast<int>(top)    / (int)TILESIZE;
        int righti  = static_cast<int>(right)  / (int)TILESIZE;
        int bottomi = static_cast<int>(bottom) / (int)TILESIZE;

        int widthi  = righti  - lefti;
        int heighti = bottomi - topi;

        Recti tsi_bbox {
            lefti,
            topi,
            widthi,
            heighti
        };

        Recti tilemap_bounds{ size_min, size_max - size_min };
        tilemap_bounds.intersects(tsi_bbox, tsi_bbox);

        return tsi_bbox;
    }

    std::optional<QuadID> ColliderTileMap::first_quad_in_rect(Rectf area, Recti& tile_area) const {
        tile_area = get_tile_area_for_rect(area);

        if (tile_area.width == 0 || tile_area.height == 0)
            return {};

        Vec2i pos = { tile_area.left, tile_area.top };

        // next quadid
        auto iterate_pos = [](Vec2i& pos, const Recti& tile_area) -> bool {
            ++pos.x;
            if (pos.x >= tile_area.left + tile_area.width) {
                pos.x = tile_area.left;
                ++pos.y;
            }
            return pos.y < tile_area.top + tile_area.height;
        };

        const ColliderQuad* quad = get_quad(pos);
        bool in_bounds = true;

        while (!quad && in_bounds) {
            in_bounds = iterate_pos(pos, tile_area);
            quad = in_bounds ? get_quad(pos) : nullptr;
        }

        return quad ? std::make_optional(quad->getID()) : std::nullopt;
    }
    std::optional<QuadID> ColliderTileMap::next_quad_in_rect(Rectf area, QuadID quadid, const Recti& tile_area) const {
        Vec2i pos = to_pos(quadid);

        // next quadid
        auto iterate_pos = [](Vec2i& pos, const Recti& tile_area) -> bool {
            ++pos.x;
            if (pos.x >= tile_area.left + tile_area.width) {
                pos.x = tile_area.left;
                ++pos.y;
            }
            return pos.y < tile_area.top + tile_area.height;
        };

        bool in_bounds = true;
        const ColliderQuad* quad = nullptr;

        while (!quad && in_bounds) {
            in_bounds = iterate_pos(pos, tile_area);
            quad = in_bounds ? get_quad(pos) : nullptr;
        }

        return quad ? std::make_optional(quad->getID()) : std::nullopt;
    }

    std::optional<QuadID> ColliderTileMap::first_quad_in_line(Linef line, Recti& tile_area) const {

        if (!boundingBox.contains(line))
            return {};

        line = math::shift(line, -getPosition());
        auto beg = line_thru_grid<float>::iterator{ line, Vec2i{ collisionMapSize }, Vec2f{TILESIZE_F, TILESIZE_F}};
        auto end = Vec2i{ line.p2 / TILESIZE_F };

        while (beg != end) {
            if (get_quad(beg->pos)) {
                // LOG_INFO("{}", beg->pos);
                return getTileID(beg->pos);
            }
            else {
                ++beg;
            }
        }

        return {};

    }
    std::optional<QuadID> ColliderTileMap::next_quad_in_line(Linef line, QuadID quadid, const Recti& tile_area) const {
        line = math::shift(line, -getPosition());
        auto beg = line_thru_grid<float>::iterator{ line, Vec2i{ collisionMapSize }, Vec2f{TILESIZE_F, TILESIZE_F}, to_pos(quadid)};
        auto end = Vec2i{ line.p2 / TILESIZE_F };

        ++beg;

        while (beg != end) {
            if (get_quad(beg->pos)) {
                // LOG_INFO("{}", beg->pos);
                return getTileID(beg->pos);
            }
            else {
                ++beg;
            }
        }

        return {};
    }

	void ColliderTileMap::updateGhosts(const Vec2i& position) {
		auto [quad, tile] = get_tile(position);
		if (!quad)
			return;

		// Rectf tileArea{ Vec2f(position * TILESIZE_F), Vec2f(TILESIZE_F, TILESIZE_F) };
		Recti adjArea{ position - Vec2i(1, 1), Vec2i(3, 3) };

        // get adjacent collider quads
        std::vector<const ColliderQuad*> nearbyQuads;
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                auto pos = Vec2i{ x, y } + Vec2i{ adjArea.getPosition() };
                if (validPosition(pos)) {
                    nearbyQuads.push_back(get_tile(pos).first);
                }
            }
        }

        // connect surfaces
		for (auto dir : direction::cardinals) {
			if (ColliderSurface* surf_ptr = quad->getSurface(dir)) {
                quad->setSurface(dir, findColliderGhosts(nearbyQuads, *surf_ptr));
			}
		}
	}

}
