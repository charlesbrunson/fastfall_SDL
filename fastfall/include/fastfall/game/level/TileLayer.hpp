#pragma once

//#include "util/Updatable.hpp"

#include "fastfall/render/Drawable.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/resource/asset/TileLayerData.hpp"

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
	static constexpr int	TILEDATA_NONE	= UINT8_MAX;
	static constexpr Vec2u	kChunkSize		= Vec2u{ GAME_TILE_W / 2u, GAME_TILE_H / 2u };

	// copy of layer from asset
	TileLayerData layer_data;

	struct tile_dyn_t {
		std::vector<uint8_t> logic_id;
	} tiles_dyn;

	struct dyn_t {
		struct parallax_dyn_t {
			Vec2f init_offset;
			Vec2f cam_factor;
			Vec2f offset;
		} parallax;

		struct scroll_dyn_t {
			Vec2f offset;
		} scroll;

		struct collision_dyn_t {
			ColliderTileMap* tilemap_ptr = nullptr;
		} collision;

		struct chunk_t {
			const TilesetAsset* tileset;
			ChunkVertexArray varray;
		};
		std::vector<chunk_t> chunks;

		std::vector<std::unique_ptr<TileLogic>> tile_logic;
	} dyn;

	GameContext m_context;
	Vec2f offset;

public:

	TileLayer(GameContext context, unsigned id, Vec2u levelsize);
	TileLayer(GameContext context, const TileLayerData& layerData);

	TileLayer(const TileLayer& tile);
	TileLayer& operator=(const TileLayer& tile);

	TileLayer(TileLayer&& tile) noexcept;
	TileLayer& operator=(TileLayer&& tile) noexcept;

	~TileLayer();

	void initFromAsset(const TileLayerData& layerData);
	void update(secs deltaTime);
	void predraw(secs deltaTime);

	void setTile(const Vec2u& position, const Vec2u& texposition, const TilesetAsset& tileset, bool useLogic = true);
	void removeTile(const Vec2u& position);
	void clear();

	void shallow_copy(const TileLayer& layer, Rectu area);

	bool set_collision(bool enabled, unsigned border = 0u);
	bool set_parallax(bool enabled, Vec2u parallax_size = Vec2u{});
	bool set_scroll(bool enabled, Vec2f rate = Vec2f{});

	inline Vec2f getOffset() const noexcept { return offset; };
	inline void  setOffset(Vec2f off) noexcept { offset = off; };

	// TileLayerData passthrough
	inline bool			hasParallax()			const { return layer_data.hasParallax();		};
	inline Vec2u		getParallaxSize()		const { return layer_data.getParallaxSize();	};

	inline bool			hasScrolling()			const { return layer_data.hasScrolling();		};
	inline Vec2f		getScrollRate()			const { return layer_data.getScrollRate();		};

	inline bool			hasCollision()			const { return layer_data.hasCollision();		};
	inline unsigned		getCollisionBorders()	const { return layer_data.getCollisionBorders();};

	inline unsigned		getID()					const { return layer_data.getID();				};
	inline Vec2u		getLevelSize()			const { return layer_data.getSize();			};

	// size varies based on parallax enabled
	inline Vec2u		getSize() const { return hasParallax() ? getParallaxSize() : getLevelSize(); };

	// tile queries
	bool hasTileAt(Vec2u tile_pos);
	std::optional<Vec2u> getTileTexPos(Vec2u tile_pos);
	const TilesetAsset* getTileTileset(Vec2u tile_pos);

	// position queries
	Vec2f getWorldPosFromTilePos(Vec2i tile_pos) const;
	std::optional<Vec2i> getTileFromWorldPos (Vec2f position) const;
	Vec2f worldToLocalCoord(Vec2f world_pos);

	// offset queries
	Vec2f get_parallax_offset() const { return dyn.parallax.offset; };
	Vec2f get_scroll_offset() const { return dyn.scroll.offset; };
	Vec2f get_total_offset() const { return dyn.parallax.offset + dyn.scroll.offset; };

	// set visibility
	bool hidden = false;

protected:

	//void populateLogic();

	bool handlePreContact(Vec2i pos, const Contact& contact, secs duration);
	void handlePostContact(Vec2i pos, const PersistantContact& contact);

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;
};

}
