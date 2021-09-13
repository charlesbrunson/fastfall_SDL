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
public:

	TileLayer(GameContext context, unsigned id, Vec2u size, bool initCollision = false);
	TileLayer(GameContext context, unsigned id, const TileLayerRef& layerData, bool initCollision = false);
	TileLayer(const TileLayer& tile);
	TileLayer(TileLayer&& tile) noexcept;
	TileLayer& operator=(const TileLayer& tile);
	TileLayer& operator=(TileLayer&& tile) noexcept;

	~TileLayer();

	void initFromAsset(const TileLayerRef& layerData, unsigned id, bool initCollision = false);

	void setTile(const Vec2u& position, const Vec2u& texposition, const TilesetAsset& tileset, bool useLogic = true);
	void removeTile(const Vec2u& position);
	void clear();

	void shallow_copy(const TileLayer& layer, Rectu area, Vec2u lvlSize);

	void set_parallax(bool enabled, Vec2u levelTileSize = Vec2u{});
	void set_scrollrate(Vec2f rate);


	void update(secs deltaTime);

	void predraw(secs deltaTime);

	inline ColliderTileMap* getCollisionMap() { return collision; };

	inline unsigned int getID() const { return layerID; };

	inline Vec2u getSize() const noexcept { return size; };
	inline Vec2f getOffset() const noexcept { return offset; };
	inline void  setOffset(Vec2f off) noexcept { offset = off; };

	inline bool isParallax() const noexcept { return has_parallax; };
	inline bool hasScrollX() const noexcept { return scrollRate.x != 0.f; };
	inline bool hasScrollY() const noexcept { return scrollRate.y != 0.f; };

	Vec2f worldToLocalCoord(Vec2f world_pos);

	void enable_collision();
	void remove_collision();

	bool has_collision() const { return hasCollision; };

	bool hidden = false;

protected:

	bool hasCollision = false;

	bool handlePreContact(Vec2i pos, const Contact& contact, secs duration);
	void handlePostContact(Vec2i pos, const PersistantContact& contact);

	GameContext m_context;

	unsigned int layerID;

	Vec2u size;
	Vec2f offset;

	// parallax data
	bool has_parallax = false;
	struct ParallaxState {
		Vec2f initOffset;
		Vec2f camFactor;
	} parallax;
	Vec2f parallax_offset;

	// scroll data
	Vec2f scroll_rollover;
	Vec2f scrollRate;
	Vec2f scroll_offset;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

	static constexpr int TILEDATA_NONE = UINT8_MAX;
	struct TileData {
		uint8_t tileset_id = TILEDATA_NONE;
		uint8_t logic_id = TILEDATA_NONE;
		Vec2u tex_pos;
		bool has_tile = false;
	};

	std::vector<TileData> pos2data;

	struct ChunkVA {
		const TilesetAsset* tileset;
		ChunkVertexArray varray;
	};
	std::vector<ChunkVA> chunks;

	ColliderTileMap* collision = nullptr;

	std::vector<std::pair<Vec2u, unsigned>> tile2logic;
	std::vector<std::unique_ptr<TileLogic>> tileLogic;
};

}