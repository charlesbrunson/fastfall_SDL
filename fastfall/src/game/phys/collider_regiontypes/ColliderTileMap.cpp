#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

#include <algorithm>
#include <stdlib.h>
#include <cmath>

#include "fastfall/render/DebugDraw.hpp"

namespace ff {

	struct SideAssociated {
		Vec2i gridoffset;
		Cardinal toCard;
		Cardinal oppositeCard;
	};

	const static SideAssociated sides[] = {
		// tile offset,   original side,   adjacent side
		{Vec2i(0, -1), Cardinal::NORTH, Cardinal::SOUTH},
		{Vec2i(1,  0), Cardinal::EAST,  Cardinal::WEST},
		{Vec2i(0,  1), Cardinal::SOUTH, Cardinal::NORTH},
		{Vec2i(-1, 0), Cardinal::WEST,  Cardinal::EAST},
	};


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

		tileCollisionMap = std::make_unique<ColliderQuad[]>(getTileIndex(size_max));
		tileShapeMap = std::make_unique<TileTable[]   >(getTileIndex(size_max));

		for (int n = 0; n < getTileIndex(size_max); n++) {
			tileCollisionMap[n].setID(n);
		}

		//debug_dirtyFlag = true;
		boundingBox = Rectf(
			Vec2f(size_min * TILESIZE_F),
			Vec2f(size_max * TILESIZE_F)
		);
	}

	void ColliderTileMap::update(secs deltaTime) {
		applyChanges();

		if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_COLLIDER)) {
			debugDrawQuad(validCollisionSize, &tileCollisionMap[0], getPosition(), this, update_debugDraw);
		}
	}

	const ColliderQuad* ColliderTileMap::get_quad(int quad_id) const noexcept
	{
		return tileShapeMap[quad_id].hasTile ? &tileCollisionMap[quad_id] : nullptr;
	}


	bool ColliderTileMap::on_precontact(int quad_id, const Contact& contact, secs duration) const {

		if (validPosition(quad_id) && callback_on_precontact) {
			Vec2i position;
			position.y = quad_id / (size_max.x);
			position.x = quad_id - (position.y * (size_max.x - size_min.x));
			position += size_min;

			return callback_on_precontact(position, contact, duration);
		}
		return true;
	}

	void ColliderTileMap::on_postcontact(int quad_id, const PersistantContact& contact) const {
		if (validPosition(quad_id) && callback_on_precontact) {
			Vec2i position;
			position.y = quad_id / (size_max.x);
			position.x = quad_id - (position.y * (size_max.x - size_min.x));
			position += size_min;

			callback_on_postcontact(position, contact);
		}
	}

	void ColliderTileMap::setBorders(const Vec2u& size, const unsigned cardinalBits) {
		if (!hasBorder)	return;

		static constexpr Cardinal sides[4] = {
			Cardinal::NORTH,
			Cardinal::EAST,
			Cardinal::SOUTH,
			Cardinal::WEST,
		};

		for (auto side : sides) {
			if (cardinalBits & cardinalBit[side]) {

				TileShape shape;
				shape.shapeTouches = cardinalBit[cardinalOpposite(side)];

				shape.type =
					((side == Cardinal::NORTH) || (side == Cardinal::SOUTH) ?
						TileShape::Type::LEVELBOUNDARY :
						TileShape::Type::LEVELBOUNDARY_WALL);

				shape.vflipped = (side == Cardinal::NORTH);
				shape.hflipped = (side == Cardinal::EAST);

				switch (side) {
				case Cardinal::NORTH: for (unsigned x = 0; x < size.x; x++) setTile(Vec2i(x, -1), shape); break;
				case Cardinal::EAST:  for (unsigned y = 0; y < size.y; y++) setTile(Vec2i(size.x, y), shape); break;
				case Cardinal::SOUTH: for (unsigned x = 0; x < size.x; x++) setTile(Vec2i(x, size.y), shape); break;
				case Cardinal::WEST:  for (unsigned y = 0; y < size.y; y++) setTile(Vec2i(-1, y), shape); break;
				}
			}
			else {
				switch (side) {
				case Cardinal::NORTH: for (unsigned x = 0; x < size.x; x++) removeTile(Vec2i(x, -1)); break;
				case Cardinal::EAST:  for (unsigned y = 0; y < size.y; y++) removeTile(Vec2i(size.x, y)); break;
				case Cardinal::SOUTH: for (unsigned x = 0; x < size.x; x++) removeTile(Vec2i(x, size.y)); break;
				case Cardinal::WEST:  for (unsigned y = 0; y < size.y; y++) removeTile(Vec2i(-1, y)); break;
				}
			}
		}
		applyChanges();
	}

	std::pair<bool, bool> ColliderTileMap::cullTouchingSurfaces(ColliderSurface& lhs, ColliderSurface& rhs) {

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


	void ColliderTileMap::clear() {
		tileCollisionMap = std::make_unique<ColliderQuad[]>(getTileIndex(size_max));
		tileShapeMap = std::make_unique<TileTable[]   >(getTileIndex(size_max));

		for (int n = 0; n < getTileIndex(size_max); n++) {
			tileCollisionMap[n].setID(n);
		}

		//touches.clear();
		//surf.clear();
		validCollisionSize = 0;
	}

	void ColliderTileMap::applyChanges() {
		if (editQueue.empty()) {
			update_debugDraw = false;
			return;
		}
		else {
			update_debugDraw = true;
		}

		Vec2i size = size_max - size_min;
		std::vector<bool> impacted(size.x * size.y);

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

			if (has_change) {
				for (int xx = pos.x - 1 - size_min.x; xx <= pos.x + 1 - size_min.x; xx++) {
					for (int yy = pos.y - 1 - size_min.y; yy <= pos.y + 1 - size_min.y; yy++) {

						if (xx >= 0 && xx < size.x &&
							yy >= 0 && yy < size.y) {

							change_min.x = std::min(xx, change_min.x);
							change_min.y = std::min(yy, change_min.y);

							change_max.x = std::max(xx, change_max.x);
							change_max.y = std::max(yy, change_max.y);

							int ndx = yy * size.x + xx;

							impacted[ndx] = true;
						}

					}
				}
			}

			editQueue.pop();
			any_change |= has_change;
		}

		if (any_change) {

			// update impacted
			for (int xx = change_min.x; xx <= change_max.x; xx++) {
				for (int yy = change_min.y; yy <= change_max.y; yy++) {

					if (impacted[(size_t)yy * size.x + xx]) {

						updateGhosts(Vec2i{ size_min.x + xx , size_min.y + yy });
					}

				}
			}
		}
	}

	bool ColliderTileMap::applyRemoveTile(const Edit& change) {

		auto [quad, tile] = getTile(change.position);

		if (!quad)
			return false;

		unsigned shapeTouchBits = tile->shape.shapeTouches;
		for (const auto& side : sides) {
			if (shapeTouchBits & cardinalBit[side.toCard]) {
				auto [quad_adj, tile_adj] = getTile(change.position + side.gridoffset);

				if (quad_adj && (tile_adj->shape.shapeTouches & cardinalBit[side.oppositeCard])) {

					ColliderQuad original = tile_adj->toQuad(getTileIndex(change.position + side.gridoffset));// (tile_adj.get().position, tile_adjacent->getShape());

					const ColliderSurface* originalSurf = original.getSurface(side.oppositeCard);
					if (originalSurf) {
						if (!quad_adj->hasAnySurface())
							validCollisionSize++;

						quad_adj->setSurface(side.oppositeCard, *originalSurf);
					}
				}
			}
		}

		if (quad->hasAnySurface()) {
			validCollisionSize--;
		}

		tileShapeMap[getTileIndex(change.position)].hasTile = false;
		return true;
	}
	bool ColliderTileMap::applySetTile(const Edit& change) {

		size_t ndx = getTileIndex(change.position);
		ColliderTile nTile(change.position, change.toShape, change.material, change.matFacing);
		ColliderQuad nQuad = nTile.toQuad(ndx);

		if (auto [quad, tile] = getTile(change.position); quad) {

			if (tile->shape.type == nTile.shape.type
				&& (tile->shape.hflipped == nTile.shape.hflipped)
				&& (tile->shape.vflipped == nTile.shape.vflipped)
				) {
				// no impact to collision map
				return false;
			}
			else {
				applyRemoveTile(Edit{ change.position, true });
			}
		}

		if (nTile.shape.type == TileShape::Type::EMPTY)
			return true;

		unsigned shapeTouchBits = nTile.shape.shapeTouches;
		for (const auto& side : sides) {
			if (shapeTouchBits & cardinalBit[side.toCard]) {


				auto [quad_adj, tile_adj] = getTile(change.position + side.gridoffset);

				if (quad_adj && (tile_adj->shape.shapeTouches & cardinalBit[side.oppositeCard])) {

					ColliderSurface* added = nQuad.getSurface(side.toCard);
					ColliderSurface* adjacent = quad_adj->getSurface(side.oppositeCard);

					if (added == nullptr || adjacent == nullptr) {
						//LOG_WARN("missing surface");
						continue;
					}

					auto [first, second] = cullTouchingSurfaces(*added, *adjacent);

					if (first) {
						nQuad.removeSurface(side.toCard);
					}
					if (second) {
						quad_adj->removeSurface(side.oppositeCard);
						if (!quad_adj->hasAnySurface()) {
							validCollisionSize--;
						}
					}
				}
			}
		}

		if (nQuad.hasAnySurface()) {
			validCollisionSize++;
		}

		tileShapeMap[ndx].hasTile = true;
		tileShapeMap[ndx].tile = nTile;
		tileCollisionMap[ndx] = nQuad;
		return true;
	}

	const ColliderQuad* ColliderTileMap::getTileCollision(const Vec2i& at) const {

		size_t ndx = getTileIndex(at);
		if (validPosition(ndx))
		{
			//auto& it = tileCollisionMap[ndx];
			return (tileShapeMap[ndx].hasTile ? &tileCollisionMap[ndx] : nullptr);
		}
		return nullptr;
	}

	void ColliderTileMap::getQuads(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& buffer) const {

		Rectf boundingBox = area;
		boundingBox.left -= getPosition().x;
		boundingBox.top -= getPosition().y;

		Recti tileArea;
		tileArea.top = static_cast<int>(floorf(boundingBox.top / TILESIZE_F));
		tileArea.left = static_cast<int>(floorf(boundingBox.left / TILESIZE_F));
		tileArea.width = static_cast<int>(ceilf((boundingBox.left + boundingBox.width) / TILESIZE_F)) - tileArea.left;
		tileArea.height = static_cast<int>(ceilf((boundingBox.top + boundingBox.height) / TILESIZE_F)) - tileArea.top;

		if (tileArea.width == 0) {
			tileArea.left -= 1;
			tileArea.width += 2;
		}
		else if (tileArea.height == 0) {
			tileArea.top -= 1;
			tileArea.height += 2;
		}

		Rectf tileBounds(Vec2f(), Vec2f(TILESIZE_F, TILESIZE_F));

		for (int yy = tileArea.top; yy < tileArea.top + tileArea.height; yy++) {
			for (int xx = tileArea.left; xx < tileArea.left + tileArea.width; xx++) {
				if (auto* tile = getTileCollision(Vec2i(xx, yy))) {
					tileBounds.left = xx * TILESIZE_F;
					tileBounds.top = yy * TILESIZE_F;

					buffer.push_back(std::make_pair(tileBounds, tile));
				}
			}
		}
	}

	void ColliderTileMap::updateGhosts(const Vec2i& position) {
		//constexpr float nan = std::numeric_limits<float>::quiet_NaN();

		auto [quad, tile] = getTile(position);
		if (!quad)
			return;

		Rectf tileArea{ Vec2f(position * TILESIZE_F), Vec2f(TILESIZE_F, TILESIZE_F) };
		Recti adjArea{ position - Vec2i(1, 1), Vec2i(3, 3) };

		int y = 0, x = 0;

		std::array<std::pair<Vec2i, const ColliderQuad*>, 9> nearbyTiles;
		for (int yy = adjArea.top; yy < adjArea.top + adjArea.height; yy++, y++) {
			for (int xx = adjArea.left; xx < adjArea.left + adjArea.width; xx++, x++) {
				Vec2i v(xx, yy);

				if (validPosition(v)) {
					nearbyTiles.at(x + (y * 3)).first = v;
					nearbyTiles.at(x + (y * 3)).second = getTile(v).first;

				}
				else {
					nearbyTiles.at(x + (y * 3)).second = nullptr;
				}
			}
			x = 0;
		}

		struct Ghosts {
			const ColliderSurface* next = nullptr;
			const ColliderSurface* prev = nullptr;
			Vec2f g0;
			Vec2f g3;
			bool g0virtual = true;
			bool g3virtual = true;
		};

		//for (auto& surface : tile->getSurfaces()) {
		for (unsigned it = CARDINAL_FIRST; it <= CARDINAL_LAST; it++) {

			if (ColliderSurface* surf_ptr = quad->getSurface(static_cast<Cardinal>(it))) {

				ColliderSurface surf = *surf_ptr;

				auto ghosts = getGhosts(nearbyTiles, surf_ptr->surface, quad->hasOneWay);
				surf.ghostp0 = ghosts.g0;
				surf.ghostp3 = ghosts.g3;
				surf.g0virtual = ghosts.g0virtual;
				surf.g3virtual = ghosts.g3virtual;
				surf.next = ghosts.next;
				surf.prev = ghosts.prev;
				quad->setSurface(static_cast<Cardinal>(it), surf);
			}
		}
	}


	ColliderTileMap::Ghosts ColliderTileMap::getGhosts(const std::array<std::pair<Vec2i, const ColliderQuad*>, 9>& nearby, const Linef& surface, bool isOneWay) {

		std::vector<const ColliderSurface*> candidatesg0;
		std::vector<const ColliderSurface*> candidatesg3;

		for (auto& tile : nearby) {

			if (!tile.second || (!isOneWay && tile.second->hasOneWay))
				continue;

			for (auto& qsurf : tile.second->surfaces) {
				if (!qsurf.hasSurface)
					continue;

				if (qsurf.collider.surface.p2 == surface.p1) {
					//candidatesg0.push_back(surface.collider.surface.p1);
					candidatesg0.push_back(&qsurf.collider);
				}
				else if (qsurf.collider.surface.p1 == surface.p2) {
					//candidatesg3.push_back(surface.collider.surface.p2);
					candidatesg3.push_back(&qsurf.collider);
				}
			}
		}
		//std::pair<Vec2f, Vec2f> r;

		auto v = math::vector(surface);

		Angle idealAng(atan2f(v.y, v.x));


		// select ghosts based on the angle diff closest to zero
		auto g0 = std::min_element(candidatesg0.begin(), candidatesg0.end(),
			//[&surf, &v, idealAng](const Vec2f& lhs, const Vec2f& rhs) -> bool {
			[&surface, &v, idealAng](const ColliderSurface* lhs, const ColliderSurface* rhs) -> bool {
				if (lhs == rhs) return false;

				auto v1 = math::vector(Linef(lhs->surface.p1, surface.p1));
				auto v2 = math::vector(Linef(rhs->surface.p1, surface.p1));

				// if comparing floor surface to ceil surface, 
				// favor the one that v also is floor or ceil

				if (v1.x != 0.f && v2.x != 0.f && v.x != 0.f
					&& ((v1.x < 0.f) != (v2.x < 0.f)))
				{
					return (v1.x < 0.f) == (v.x < 0.f);
				}

				// otherwise compare by angle
				Angle lhsAng(atan2f(v1.y, v1.x));
				Angle rhsAng(atan2f(v2.y, v2.x));
				return abs((lhsAng - idealAng).radians()) < abs((rhsAng - idealAng).radians());

			});

		auto g3 = std::min_element(candidatesg3.begin(), candidatesg3.end(),
			//[surf, v, idealAng](const Vec2f& lhs, const Vec2f& rhs) -> bool {
			[&surface, &v, idealAng](const ColliderSurface* lhs, const ColliderSurface* rhs) -> bool {
				if (lhs == rhs) return false;

				auto v1 = math::vector(Linef(surface.p2, lhs->surface.p2));
				auto v2 = math::vector(Linef(surface.p2, rhs->surface.p2));

				// if comparing floor surface to ceil surface, 
				// favor the one that v also is floor or ceil

				if (v1.x != 0.f && v2.x != 0.f && v.x != 0.f
					&& ((v1.x < 0.f) != (v2.x < 0.f)))
				{
					return (v1.x < 0.f) == (v.x < 0.f);
				}

				// otherwise compare by angle
				Angle lhsAng(atan2f(v1.y, v1.x));
				Angle rhsAng(atan2f(v2.y, v2.x));

				return abs((lhsAng - idealAng).radians()) < abs((rhsAng - idealAng).radians());

			});

		Ghosts r;
		r.g0virtual = (g0 == candidatesg0.end());
		if (r.g0virtual) {
			r.prev = nullptr;
			r.g0 = surface.p1 - v;
		}
		else {
			r.prev = *g0;
			r.g0 = r.prev->surface.p1;
		}
		//r.g0 = (r.g0virtual ? surf.p1 - v : *g0);

		r.g3virtual = (g3 == candidatesg3.end());
		if (r.g3virtual) {
			r.next = nullptr;
			r.g3 = surface.p2 + v;
		}
		else {
			r.next = *g3;
			r.g3 = r.next->surface.p2;
		}
		//r.g3 = (r.g3virtual ? surf.p2 + v : *g3);
		return r;
	}

}