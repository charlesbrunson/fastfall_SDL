#pragma once

//#include "util/Updatable.hpp"

#include "fastfall/render/Drawable.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/render/TileVertexArray.hpp"

//#include <SFML/Graphics.hpp>
#include <memory>

namespace ff {

class TileLayer : public Drawable {
public:
	TileLayer(const LayerRef& layerData, bool initCollision = false);
	TileLayer(const TileLayer& tile);
	TileLayer(TileLayer&& tile) noexcept;
	TileLayer& operator=(const TileLayer& tile);
	TileLayer& operator=(TileLayer&& tile) noexcept;

	void initFromAsset(const LayerRef& layerData, bool initCollision = false);

	void setTile(const Vec2u& position, const Vec2u& texposition, const TilesetAsset& tileset);
	void removeTile(const Vec2u& position);
	void clear();

	void update(secs deltaTime);

	void predraw(secs deltaTime);

	inline std::shared_ptr<ColliderTileMap>& getCollisionMap() { return collision; };

	inline unsigned int getID() { return layerID; };

	inline Vec2u getSize() const noexcept { return size; };
	inline Vec2f getOffset() const noexcept { return offset; };
	inline void  setOffset(Vec2f off) noexcept { offset = off; };

	bool hidden = false;
	bool hasCollision = false;
	//bool showCollision = false;

protected:

	struct TileTimer {
		Vec2u tex_position;
		secs time_to_anim;
		Vec2u tile_impacted;
	};

	std::vector<TileTimer> tile_timers;

	unsigned int layerID;

	const LayerRef* ref;

	Vec2u size;
	Vec2f offset;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

	//sets of vertexs for each texture this layer needs
	std::map<Vec2u, const TilesetAsset*> pos2tileset;
	std::map<const TilesetAsset*, TileVertexArray> tileVertices;
	std::shared_ptr<ColliderTileMap> collision;
};

}