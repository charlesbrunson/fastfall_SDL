#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/util/log.hpp"

#include <assert.h>

//#include <SFML/Graphics.hpp>

namespace ff {

//constexpr unsigned vertsPerTile = 6u;

/*
void updateTileVertices(VertexArray& vtx, unsigned ndx, const Vec2u& texPos, const Vec2i& gamePos) {


	assert(vtx.getPrimitiveType() == sf::PrimitiveType::Quads);
	assert(ndx + vertsPerTile <= vtx.getVertexCount());

	// update texCoords
	vtx[ndx + 0].texCoords = (texPos + Vec2u(0, 0)) * TILESIZE_F;
	vtx[ndx + 1].texCoords = (texPos + Vec2u(1, 0)) * TILESIZE_F;
	vtx[ndx + 2].texCoords = (texPos + Vec2u(1, 1)) * TILESIZE_F;
	vtx[ndx + 3].texCoords = (texPos + Vec2u(0, 1)) * TILESIZE_F;

	// update worldposition
	vtx[ndx + 0].position = (gamePos + Vec2u(0, 0)) * TILESIZE_F;
	vtx[ndx + 1].position = (gamePos + Vec2u(1, 0)) * TILESIZE_F;
	vtx[ndx + 2].position = (gamePos + Vec2u(1, 1)) * TILESIZE_F;
	vtx[ndx + 3].position = (gamePos + Vec2u(0, 1)) * TILESIZE_F;

}
*/

TileLayer::TileLayer(const LayerRef& layerData, bool initCollision) {

	initFromAsset(layerData, initCollision);
}
TileLayer::TileLayer(const TileLayer& tile) {
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	collision = tile.collision;
	tileVertices = tile.tileVertices;
	pos2tileset = tile.pos2tileset;
	tile_timers = tile.tile_timers;
}
TileLayer& TileLayer::operator=(const TileLayer& tile) {
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	collision = tile.collision;
	tileVertices = tile.tileVertices;
	pos2tileset = tile.pos2tileset;
	tile_timers = tile.tile_timers;
	return *this;
}
TileLayer::TileLayer(TileLayer&& tile) noexcept {
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2tileset, tile.pos2tileset);
	tileVertices.swap(tile.tileVertices);
	tile_timers.swap(tile.tile_timers);
}
TileLayer& TileLayer::operator=(TileLayer&& tile) noexcept {
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2tileset, tile.pos2tileset);
	tileVertices.swap(tile.tileVertices);
	tile_timers.swap(tile.tile_timers);
	return *this;
}

void TileLayer::initFromAsset(const LayerRef& layerData, bool initCollision) {
	clear();
	assert(layerData.type == LayerType::TILELAYER);

	ref = &layerData;
	hasCollision = initCollision;
	layerID = layerData.id;

	size = layerData.tileLayer->tileSize;

	if (hasCollision)
		collision = std::make_shared<ColliderTileMap>(Vec2i(size.x, size.y), true);

	for (const auto& i : layerData.tileLayer->tiles) {
		TilesetAsset* ta = Resources::get<TilesetAsset>(*i.second.tilesetName);
		if (ta) {
			setTile(i.first, i.second.texPos, *ta);
		}
		else {
			LOG_ERR_("unknown tileset {}", *i.second.tilesetName);
		}
	}
	if (hasCollision)
		collision->applyChanges();

}


void TileLayer::update(secs deltaTime) {

}

void TileLayer::predraw(secs deltaTime) {

	bool changedTile = false;

	for (auto& timer : tile_timers) {
		timer.time_to_anim -= deltaTime;

		if (timer.time_to_anim < 0.0) {
			changedTile = true;
		}
	}

	if (changedTile) {
		auto iter = std::partition(tile_timers.begin(), tile_timers.end(), [](const TileTimer& timer) {
			return timer.time_to_anim > 0.0;
			});

		if (iter != tile_timers.end()) {
			std::vector<TileTimer> copy_end{ iter, tile_timers.end() };

			tile_timers.erase(iter, tile_timers.end());

			for (auto& timer : copy_end) {

				auto* tileset = pos2tileset.at(timer.tile_impacted);
				Tile t = tileset->getTile(timer.tex_position);

				Vec2u tex_pos;

				if (t.has_next_tileset()) {
					std::string_view next_name = tileset->getTilesetRef(t.next_tileset);
					tileset = Resources::get<TilesetAsset>(next_name);

					// if different tileset, use as absolute
					tex_pos = t.next_offset;
				}
				else {
					// if same tileset, use as relative
					tex_pos = timer.tex_position + t.next_offset;
				}
				setTile(timer.tile_impacted, tex_pos, *tileset);

			}
		}
	}

	if (changedTile) {
		collision->applyChanges();
	}
}

void TileLayer::setTile(const Vec2u& position, const Vec2u& texposition, const TilesetAsset& tileset) {

	auto iter1 = pos2tileset.find(position);
	if (iter1 != pos2tileset.end()) {
		auto iter2 = tileVertices.find(iter1->second);
		if (iter2 != tileVertices.end()) {
			iter2->second.blank(position);
		}
	}

	auto vertarr = tileVertices.find(&tileset);
	if (vertarr == tileVertices.end()) {
		auto [iter, inserted] = tileVertices.insert(std::make_pair(&tileset, TileVertexArray(size)));
		iter->second.setTexture(tileset.getTexture());
		iter->second.setTile(position, texposition);
	}
	else {
		vertarr->second.setTile(position, texposition);
	}
	pos2tileset[position] = &tileset;


	if (hasCollision) {
		collision->setTile(Vec2i(position), tileset.getTile(texposition).shape);
	}

	Tile t = tileset.getTile(texposition);

	if (t.has_animation()) {
		tile_timers.push_back(TileTimer{});
		tile_timers.back().time_to_anim = ms_to_secs(t.durationMS);
		tile_timers.back().tile_impacted = position;
		tile_timers.back().tex_position = texposition;
	}
}
void TileLayer::removeTile(const Vec2u& position) {
	auto iter1 = pos2tileset.find(position);
	if (iter1 != pos2tileset.end()) {
		auto iter2 = tileVertices.find(iter1->second);
		if (iter2 != tileVertices.end()) {
			iter2->second.erase(position);

			if (iter2->second.empty()) {
				tileVertices.erase(iter2);
			}
		}
	}
	if (hasCollision) {
		collision->removeTile(Vec2i(position));
	}
}
void TileLayer::clear() {
	ref = nullptr;
	pos2tileset.clear();
	tileVertices.clear();
	if (collision)
		collision->clear();
}

void TileLayer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform.translate(offset);

	if (!hidden) {
		for (auto& layer : tileVertices) {
			states.texture = &layer.first->getTexture();
			target.draw(layer.second, states);
		}
	}
}

}