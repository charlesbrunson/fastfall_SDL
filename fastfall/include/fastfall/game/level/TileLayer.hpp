#pragma once

//#include "util/Updatable.hpp"

#include "fastfall/render/Drawable.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
//#include "fastfall/render/TileVertexArray.hpp"
#include "fastfall/render/ChunkVertexArray.hpp"

#include "fastfall/game/GameContext.hpp"

#include "fastfall/game/level/TileLogic.hpp"

//#include <SFML/Graphics.hpp>
#include <memory>

namespace ff {

class TileLayer : public Drawable {
private:
	static constexpr int TILEDATA_NONE = UINT8_MAX;

	struct TileData {
		TileData(size_t size)
			: tile_count(size)
			, has_tile(size, false)
			, tex_pos(size)
			, tileset_id(size, TILEDATA_NONE)
			, logic_id(size, TILEDATA_NONE)
		{
		}
		TileData() = default;
		TileData(const TileData&) = default;
		TileData(TileData&&) = default;
		TileData& operator= (const TileData&) = default;
		TileData& operator= (TileData&&) = default;

		size_t tile_count = 0;
		inline unsigned size() { return tile_count; };

		inline void clear(unsigned ndx) {
			has_tile[ndx] = false;
			tex_pos[ndx] = Vec2u{ 0, 0 };
			tileset_id[ndx] = TILEDATA_NONE;
			logic_id[ndx] = TILEDATA_NONE;
		}

		std::vector<bool> has_tile;
		std::vector<Vec2u> tex_pos;
		std::vector<uint8_t> tileset_id;	// ndx corresponds to chunks
		std::vector<uint8_t> logic_id;		// ndx corresponds to tileLogic
	};

public:

	TileLayer(GameContext context, unsigned id, Vec2u levelsize);
	TileLayer(GameContext context, unsigned id, const TileLayerData& layerData);

	TileLayer(const TileLayer& tile);
	TileLayer& operator=(const TileLayer& tile);

	TileLayer(TileLayer&& tile) noexcept;
	TileLayer& operator=(TileLayer&& tile) noexcept;

	~TileLayer();

	void initFromAsset(const TileLayerData& layerData, unsigned id);

	void setTile(const Vec2u& position, const Vec2u& texposition, const TilesetAsset& tileset, bool useLogic = true);
	void removeTile(const Vec2u& position);
	void clear();

	void shallow_copy(const TileLayer& layer, Rectu area, Vec2u lvlSize);

	bool set_collision(bool enabled, unsigned border = 0u);
	bool set_parallax(bool enabled, Vec2u parallax_size = Vec2u{});
	bool set_scroll(bool enabled, Vec2f rate = Vec2f{});

	void update(secs deltaTime);
	void predraw(secs deltaTime);

	inline unsigned int getID() const { return layerID; };

	inline Vec2u get_level_size() const noexcept { return level_size; };
	inline Vec2u get_size() const noexcept { return parallax.enabled ? parallax.size : level_size; };

	inline Vec2f getOffset() const noexcept { return offset; };
	inline void  setOffset(Vec2f off) noexcept { offset = off; };

	inline bool has_parallax() const noexcept { return parallax.enabled; };
	inline Vec2u get_parallax_size() const noexcept { return parallax.size; };

	inline bool has_scroll() const noexcept { return scroll.enabled; };
	inline Vec2f get_scrollrate() const noexcept { return scroll.rate; };

	inline bool has_collision() const { return collision.enabled; };
	inline unsigned get_collision_border() const { return collision.border; };
	inline ColliderTileMap* getCollisionMap() { return collision.tilemap_ptr; };

	inline TileData& getTileData() { return tiles; };

	bool hasTileAt(Vec2u tile_pos);
	std::optional<Vec2u> getTileTexPos(Vec2u tile_pos);
	const TilesetAsset* getTileTileset(Vec2u tile_pos);

	Vec2f getWorldPosFromTilePos(Vec2i tile_pos) const;
	std::optional<Vec2i> getTileFromWorldPos (Vec2f position) const;


	Vec2f worldToLocalCoord(Vec2f world_pos);

	Vec2f get_parallax_offset() const { return parallax.offset; };
	Vec2f get_scroll_offset() const { return scroll.offset; };
	Vec2f get_total_offset() const { return parallax.offset + scroll.offset; };

	bool hidden = false;

protected:

	bool handlePreContact(Vec2i pos, const Contact& contact, secs duration);
	void handlePostContact(Vec2i pos, const PersistantContact& contact);

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

	// layer properties
	GameContext m_context;
	unsigned int layerID;

	Vec2u level_size;
	Vec2f offset;

	// parallax data
	struct parallax_t {
		bool  enabled = false;
		Vec2u size;
		Vec2f init_offset;
		Vec2f cam_factor;
		Vec2f offset;
	} parallax;

	// scroll data
	struct scroll_t {
		bool enabled = false;
		Vec2f rate;
		Vec2f offset;
	} scroll;

	// collision
	struct collision_t {
		bool enabled = false;
		unsigned border = 0u;
		ColliderTileMap* tilemap_ptr = nullptr;
	} collision;


	// tile data
	struct TileData tiles;

	// chunk vertex arrays
	struct ChunkVA {
		const TilesetAsset* tileset;
		ChunkVertexArray varray;
	};
	std::vector<ChunkVA> chunks;

	// logic
	std::vector<std::unique_ptr<TileLogic>> tileLogic;

	constexpr static Vec2u kChunkSize = Vec2u{ GAME_TILE_W / 2u, GAME_TILE_H / 2u };
};

}
