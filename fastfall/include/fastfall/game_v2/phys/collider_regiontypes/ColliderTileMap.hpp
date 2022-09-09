#pragma once

#include "fastfall/game_v2/phys/ColliderRegion.hpp"
#include "fastfall/game_v2/phys/collider_coretypes/ColliderTile.hpp"
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
		Edit(
			Vec2i pos, 
			bool remove, 
			TileShape shape = TileShape(),
			const TileMaterial* mat = nullptr,
			Cardinal _matFacing = Cardinal::N
		)	
			: position(pos)
			, removal(remove)
			, toShape(shape)
			, material(mat)
			, matFacing(_matFacing)
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
	
	struct SideAssociated {
		Vec2i gridoffset;
		Cardinal toCard;
		Cardinal oppositeCard;
	};

	struct Ghosts {
		const ColliderSurface* next = nullptr;
		const ColliderSurface* prev = nullptr;
		Vec2f g0;
		Vec2f g3;
		bool g0virtual = true;
		bool g3virtual = true;
	};

public:
	ColliderTileMap(Vec2i size, bool border = false);

	void update(secs deltaTime) override;

	const ColliderQuad* get_quad(QuadID quad_id) const noexcept override;
	const ColliderQuad* get_quad(const Vec2i& at) const noexcept;


	void setBorders(const Vec2u& size, const unsigned cardinalBits);

	inline void setTile(const Vec2i& at, const TileShape& toShape, const TileMaterial* mat = nullptr, Cardinal matFacing = Cardinal::N)
	{ 
		editQueue.push(Edit{ at, false, toShape, mat, matFacing }); 
	};
	inline void removeTile(const Vec2i& at) 
	{ 
		editQueue.push(Edit{ at, true }); 
	};
	void clear();
	void applyChanges();


	void get_quads_in_rect(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& out_buffer) const override;

	bool on_precontact(QuadID quad_id, const Contact& contact, secs duration) const override;
	void on_postcontact(const PersistantContact& contact) const override;

	void set_on_precontact(std::function<bool(Vec2i, const Contact&, secs)> func) {
		callback_on_precontact = func;
	}
	void set_on_postcontact(std::function<void(Vec2i, const PersistantContact&)> func) {
		callback_on_postcontact = func;
	}

private:


	void updateGhosts(const Vec2i& position);

	Ghosts getGhosts(const std::array<std::pair<Vec2i, const ColliderQuad*>, 9>& nearby, const Linef& surface, bool isOneWay);

	bool applyRemoveTile(const Edit& change);
	bool applySetTile(const Edit& change);

	bool validPosition(const Vec2i& at) const noexcept;
	bool validPosition(QuadID ndx) const noexcept;
	QuadID getTileID(const Vec2i& at) const noexcept;

	std::pair<ColliderQuad*, const ColliderTile*> get_tile(const Vec2i& at);

	template<typename Callable>
	void for_adjacent_touching_quads(const ColliderTile& tile, Callable&& call)
	{
		using namespace direction;

		TileTouch touch{ tile.shape };
		for (auto dir : cardinals)
		{
			if (touch.get_edge(dir) > 0)
			{
				auto [quad_adj, tile_adj] = get_tile(tile.position + to_vector<int>(dir));
				
				if (quad_adj 
					&& (touch.get_edge(dir) & TileTouch { tile_adj->shape }.get_edge(opposite(dir))) > 0)
				//if (quad_adj && TileTouch{ tile_adj->shape }.get_edge(opposite(dir)) > 0)
				{
					call(*quad_adj, *tile_adj, dir);
				}
			}
		}
	}

	void incr_valid_collision() {
		validCollisionSize++;
		update_debugDraw = true;
	}
	void decr_valid_collision() {
		validCollisionSize--;
		update_debugDraw = true;
	}

	bool update_debugDraw = false;

	bool hasBorder;
	size_t validCollisionSize = 0;

	std::function<bool(Vec2i, const Contact&, secs)> callback_on_precontact;
	std::function<void(Vec2i, const PersistantContact&)> callback_on_postcontact;

	grid_vector<ColliderQuad>	tileCollisionMap;
	grid_vector<TileTable>		tileShapeMap;

	Vec2i size_min;
	Vec2i size_max;
	Vec2u collisionMapSize;
	size_t minIndex;
	size_t maxIndex;

	std::queue<ColliderTileMap::Edit> editQueue;
};

}