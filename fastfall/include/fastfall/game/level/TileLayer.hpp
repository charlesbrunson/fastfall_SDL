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
		uint8_t tileset_id = TILEDATA_NONE;
		uint8_t logic_id = TILEDATA_NONE;
		Vec2u tex_pos;
		bool has_tile = false;
	};
public:

	TileLayer(GameContext context, unsigned id, Vec2u size);
	TileLayer(GameContext context, unsigned id, const TileLayerRef& layerData);

	TileLayer(const TileLayer& tile);
	TileLayer& operator=(const TileLayer& tile);

	TileLayer(TileLayer&& tile) noexcept;
	TileLayer& operator=(TileLayer&& tile) noexcept;

	~TileLayer();

	void initFromAsset(const TileLayerRef& layerData, unsigned id);

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

	inline Vec2u getSize() const noexcept { return size; };
	inline Vec2f getOffset() const noexcept { return offset; };
	inline void  setOffset(Vec2f off) noexcept { offset = off; };

	inline bool has_parallax() const noexcept { return parallax.enabled; };
	inline Vec2u get_parallax_size() const noexcept { return parallax.size; };

	inline bool has_scroll() const noexcept { return scroll.enabled; };
	inline Vec2f get_scrollrate() const noexcept { return scroll.rate; };

	inline bool has_collision() const { return collision.enabled; };
	inline unsigned get_collision_border() const { return collision.border; };
	inline ColliderTileMap* getCollisionMap() { return collision.tilemap_ptr; };

	inline std::vector<TileData>& getTileData() { return pos2data; };

	bool hasTileAt(Vec2u tile_pos);
	std::optional<Vec2u> getTileTexPos(Vec2u tile_pos);
	const TilesetAsset* getTileTileset(Vec2u tile_pos);

	Vec2f worldToLocalCoord(Vec2f world_pos);

	bool hidden = false;

protected:

	bool handlePreContact(Vec2i pos, const Contact& contact, secs duration);
	void handlePostContact(Vec2i pos, const PersistantContact& contact);

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

	// layer properties
	GameContext m_context;
	unsigned int layerID;

	Vec2u size;
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
		//Vec2f rollover;
	} scroll;

	// collision
	struct collision_t {
		bool enabled = false;
		unsigned border = 0u;
		ColliderTileMap* tilemap_ptr = nullptr;
	} collision;

	// tile data
	std::vector<TileData> pos2data;

	// chunk vertex arrays
	struct ChunkVA {
		const TilesetAsset* tileset;
		ChunkVertexArray varray;
	};
	std::vector<ChunkVA> chunks;

	// logic
	std::vector<std::pair<Vec2u, unsigned>> tile2logic;
	std::vector<std::unique_ptr<TileLogic>> tileLogic;
};

}
