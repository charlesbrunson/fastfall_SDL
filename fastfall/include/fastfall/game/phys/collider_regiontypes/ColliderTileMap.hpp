#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderTile.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/grid_vector.hpp"

#include <functional>
#include <array>
#include <queue>

namespace ff {

class ColliderTileMap : public ColliderRegion {
private:
	struct Edit {
		Edit(Vec2i pos, 
			bool remove, 
			TileShape shape = TileShape(),
			const TileMaterial* mat = nullptr,
			Cardinal _matFacing = Cardinal::NORTH
		) :
			position(pos),
			removal(remove),
			toShape(shape),
			material(mat),
			matFacing(_matFacing)
		{

		}

		Vec2i position;
		bool removal;
		TileShape toShape;
		const TileMaterial* material;
		Cardinal matFacing;

	};

	struct TileTable {
		bool hasTile = false;
		ColliderTile tile;
	};


public:
	ColliderTileMap(Vec2i size, bool border = false);

	void update(secs deltaTime) override;

	const ColliderQuad* get_quad(int quad_id) const noexcept override;

	void setBorders(const Vec2u& size, const unsigned cardinalBits);
	//inline void setTile(const Vec2i& at, const TileShape& toShape) { editQueue.push(Edit{ at, false, toShape }); };
	inline void setTile(
		const Vec2i& at, 
		const TileShape& toShape, 
		const TileMaterial* mat = nullptr, 
		Cardinal matFacing = Cardinal::NORTH
	) 
	{ 
		editQueue.push(Edit{ at, false, toShape, mat, matFacing }); 
	};


	inline void removeTile(const Vec2i& at) { editQueue.push(Edit{ at, true }); };
	void clear();
	void applyChanges();

	const ColliderQuad* getTileCollision(const Vec2i& at) const;

	void for_each_quad(std::function<void(const ColliderQuad&)> f) {
		for (size_t ndx = minIndex; ndx <= maxIndex; ndx++) {
			if (tileShapeMap[ndx].hasTile) f(tileCollisionMap[ndx]);
		}
	}

	void getQuads(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& buffer) const override;

	bool on_precontact(int quad_id, const Contact& contact, secs duration) const override;
	void on_postcontact(int quad_id, const PersistantContact& contact) const override;

	void set_on_precontact(std::function<bool(Vec2i, const Contact&, secs)> func) {
		callback_on_precontact = func;
	}
	void set_on_postcontact(std::function<void(Vec2i, const PersistantContact&)> func) {
		callback_on_postcontact = func;
	}

private:

	std::function<bool(Vec2i, const Contact&, secs)> callback_on_precontact;
	std::function<void(Vec2i, const PersistantContact&)> callback_on_postcontact;

	void updateGhosts(const Vec2i& position);

	struct Ghosts {
		const ColliderSurface* next = nullptr;
		const ColliderSurface* prev = nullptr;
		Vec2f g0;
		Vec2f g3;
		bool g0virtual = true;
		bool g3virtual = true;
	};
	Ghosts getGhosts(const std::array<std::pair<Vec2i, const ColliderQuad*>, 9>& nearby, const Linef& surface, bool isOneWay);

	bool applyRemoveTile(const Edit& change);
	bool applySetTile(const Edit& change);

	//returns indicate of which surfaces to be erased (corresponds to params)
	std::pair<bool, bool> cullTouchingSurfaces(ColliderSurface& lhs, ColliderSurface& rhs);

	inline bool validPosition(const Vec2i& at) const noexcept {

		return at.x >= size_min.x && at.x < size_max.x
			&& at.y >= size_min.y && at.y < size_max.y;

		//return validPosition(getTileIndex(at));
	}
	inline bool validPosition(size_t ndx) const noexcept {
		return ndx >= minIndex && ndx <= maxIndex;
	}
	
	inline size_t getTileIndex(const Vec2i& at) const noexcept {
		return (at.x - size_min.x) + ((at.y - size_min.y) * collisionMapSize.x);
	}

	inline std::pair<ColliderQuad*, const ColliderTile*> getTile(const Vec2i& at) {
		size_t ndx = getTileIndex(at);

		if (validPosition(ndx)) {
			auto& it = tileShapeMap[ndx];
			return it.hasTile ? std::make_pair(&tileCollisionMap[ndx], &it.tile) : std::make_pair(nullptr, &tileShapeMap[ndx].tile);
		}
		return std::make_pair(nullptr, nullptr);
	};

	bool update_debugDraw = false;

	bool hasBorder;
	size_t validCollisionSize = 0;
	void incr_valid_collision() {
		validCollisionSize++;
		update_debugDraw = true;
	}
	void decr_valid_collision() {
		validCollisionSize--;
		update_debugDraw = true;
	}

	//std::unique_ptr<ColliderQuad[]> tileCollisionMap;
	//std::unique_ptr<TileTable[]>    tileShapeMap;

	grid_vector<ColliderQuad> tileCollisionMap;
	grid_vector<TileTable> tileShapeMap;

	Vec2i size_min;
	Vec2i size_max;
	Vec2u collisionMapSize;
	size_t minIndex;
	size_t maxIndex;

	std::queue<ColliderTileMap::Edit> editQueue;
};

}