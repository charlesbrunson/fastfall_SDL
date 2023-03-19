#pragma once

#include "fastfall/engine/config.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/resource/asset/TileLayerData.hpp"

#include "fastfall/render/drawable/Drawable.hpp"
#include "fastfall/render/drawable/ChunkVertexArray.hpp"

#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/level/TileLogic.hpp"
#include "fastfall/game/scene/SceneConfig.hpp"
#include "fastfall/game/tile/Tile.hpp"
#include "fastfall/game/Actor.hpp"

#include <memory>
#include <set>
#include <numeric>

namespace ff {

class World;
class Level;

/*

// TilesDynamic

struct TileAnimator {
    uint8_t framecount = 1;
    uint8_t framedelay = 60;
    uint8_t curr_frame = 0;
    uint8_t framebuffer = 0;
    std::set<Vec2u> tiles;
    bool apply_tiles = true;

    void update(size_t frame_diff) {
        framebuffer += frame_diff;
        while (framebuffer > framedelay) {
            framebuffer -= framedelay;
            ++curr_frame;
            curr_frame %= framecount;
            apply_tiles = true;
        }
    }
};

struct DynTile {
    ID<TileLogic> logic;
    ID<TileAnimator> timer_id;
};

struct TilesDynamic {
    grid_vector<DynTile> tiles_dyn;
};
// TileParallax
struct TileParallax {
    Vec2f init_offset;
    Vec2f cam_factor;
    Vec2f offset;
};
// TileScroll
struct TileScroll {
    Vec2f prev_offset;
    Vec2f offset;
};
// ColliderTileMap
// TileLogic

*/

class TileLayer : public Actor {
private:
	static constexpr int	TILEDATA_NONE	= std::numeric_limits<uint8_t>::max();
	static constexpr Vec2u	kChunkSize		= Vec2u{ GAME_TILE_W / 2u, GAME_TILE_H / 2u };

	// copy of layer from asset
	TileLayerData layer_data;

	struct TileDynamic {
		uint8_t logic_id   = TILEDATA_NONE;
        uint8_t timer_id   = TILEDATA_NONE;
	};

	grid_vector<TileDynamic> tiles_dyn;

	struct dyn_t {
		struct parallax_dyn_t {
			Vec2f init_offset;
			Vec2f cam_factor;
			Vec2f offset;
		} parallax;

		struct scroll_dyn_t {
			Vec2f prev_offset;
			Vec2f offset;
		} scroll;

		struct collision_dyn_t {
            std::optional<ID<ColliderTileMap>> collider;
		} collision;

        struct frame_timer_t {
            uint8_t framecount = 1;
            uint8_t framedelay = 60;
            uint8_t curr_frame = 0;
            uint8_t framebuffer = 0;
            std::set<Vec2u> tiles;
            bool apply_tiles = true;

            void update(size_t frame_diff) {
                framebuffer += frame_diff;
                while (framebuffer > framedelay) {
                    framebuffer -= framedelay;
                    ++curr_frame;
                    curr_frame %= framecount;
                    apply_tiles = true;
                }
            }
        };

		std::vector<copyable_unique_ptr<TileLogic>> tile_logic;
        std::vector<frame_timer_t>                  timers;
		std::vector<ID<ChunkVertexArray>>           chunks;

        size_t frame_count  = 0;            // total frame count
        secs   frame_buffer = secs{ 0.0 };  // time until next frame
	} dyn;

    static constexpr secs FrameTime = secs{ 1.0 / 60.0 };

public:
	TileLayer(ActorInit init, unsigned id, Vec2u levelsize);
	TileLayer(ActorInit init, const TileLayerData& layerData);

	void initFromAsset(World& world, const TileLayerData& layerData);
	void update(World& world, secs deltaTime) override;
	void predraw(World& world, float interp, bool updated);

	void setTile(World& world, const Vec2u& position, TileID tile_id, const TilesetAsset& tileset);
	void removeTile(World& world, const Vec2u& position);

    void steal_tiles(World& w, TileLayer& from, Recti area);

	void shallow_copy(World& world, const TileLayer& src, Rectu src_area, Vec2u dst);

	bool set_collision(World& world, bool enabled, unsigned border = 0u);
	bool set_parallax(World& world, bool enabled, Vec2u parallax_size = Vec2u{});
	bool set_scroll(World& world, bool enabled, Vec2f rate = Vec2f{});

	// TileLayerData passthrough
	inline bool			hasParallax()			const { return layer_data.hasParallax();		 };
	inline Vec2u		getParallaxSize()		const { return layer_data.getParallaxSize();	 };
	inline bool			hasScrolling()			const { return layer_data.hasScrolling();		 };
	inline Vec2f		getScrollRate()			const { return layer_data.getScrollRate();		 };
	inline bool			hasCollision()			const { return layer_data.hasCollision();		 };
	inline unsigned		getCollisionBorders()	const { return layer_data.getCollisionBorders(); };
	inline unsigned		getID()					const { return layer_data.getID();				 };
	inline Vec2u		getLevelSize()			const { return layer_data.getSize();			 };
	inline std::string_view getName()			const { return layer_data.getName();			 };

	// size varies based on parallax enabled
	inline Vec2u		getSize() const { return hasParallax() ? getParallaxSize() : getLevelSize(); };

	// tile queries
	bool hasTileAt(Vec2u tile_pos) const;
	std::optional<TileID> getTileID(Vec2u tile_pos) const;
	std::optional<TileID> getTileBaseID(Vec2u tile_pos) const;
	const TilesetAsset* getTileTileset(Vec2u tile_pos) const;
	bool isTileAuto(Vec2u tile_pos) const;
	TileShape getTileShape(Vec2u tile_pos) const;

	// position queries
	struct world_pos_t {
		bool mirrorx = false;
		bool mirrory = false;
		Vec2f real;
	};
	world_pos_t getWorldPosFromTilePos(Vec2i tile_pos) const;
	std::optional<Vec2i> getTileFromWorldPos (Vec2f position) const;
	Vec2f worldToLocalCoord(Vec2f world_pos);

	// offset queries
	Vec2f get_parallax_offset() const { return dyn.parallax.offset; };
	Vec2f get_scroll_offset() const { return dyn.scroll.offset; };
	Vec2f get_total_offset() const { return dyn.parallax.offset + dyn.scroll.offset; };

	// set visibility
	bool hidden = false;

	void set_layer(World& world, scene_layer lyr);
	scene_layer get_layer() const { return layer; }

    ID<AttachPoint> get_attach_id() const {
        return attach_id;
    }

    const TileLayerData& getData() const { return layer_data; }

    void set_autotile_substitute(TileShape sub) noexcept { layer_data.set_autotile_substitute(sub); }
    TileShape get_autotile_substitute() const noexcept { return layer_data.get_autotile_substitute(); }

protected:
	scene_layer layer;
    ID<AttachPoint> attach_id;
    ColliderTileMap* get_collider(World& world);

    TileID getDisplayTileID(Vec2u tile_pos, TileID id); // need to check timer to offset

	void updateTile(
            World& world,
            const Vec2u& at,
            uint8_t prev_tileset_ndx,
            const TilesetAsset* next_tileset);
};

}
