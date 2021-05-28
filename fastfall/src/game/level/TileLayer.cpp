#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/RenderTarget.hpp"

#include "fastfall/game/GameCamera.hpp"

#include <assert.h>

namespace ff {


TileLayer::TileLayer(const LayerRef& layerData, GameContext context, bool initCollision)
	: m_context(context)
{
	initFromAsset(layerData, initCollision);
}

TileLayer::TileLayer(const TileLayer& tile)
	: m_context(tile.m_context) 
{
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	collision = tile.collision;
	tileVertices = tile.tileVertices;
	pos2tileset = tile.pos2tileset;
	tile_timers = tile.tile_timers;
	isParallax = tile.isParallax;
	parallax = tile.parallax;
}
TileLayer& TileLayer::operator=(const TileLayer& tile) {
	m_context = tile.m_context;
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	collision = tile.collision;
	tileVertices = tile.tileVertices;
	pos2tileset = tile.pos2tileset;
	tile_timers = tile.tile_timers;
	isParallax = tile.isParallax;
	parallax = tile.parallax;
	return *this;
}
TileLayer::TileLayer(TileLayer&& tile) noexcept
	: m_context(tile.m_context)
{
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2tileset, tile.pos2tileset);
	tileVertices.swap(tile.tileVertices);
	tile_timers.swap(tile.tile_timers);
	isParallax = tile.isParallax;
	std::swap(parallax, tile.parallax);
}
TileLayer& TileLayer::operator=(TileLayer&& tile) noexcept {
	m_context = tile.m_context;
	layerID = tile.layerID;
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2tileset, tile.pos2tileset);
	tileVertices.swap(tile.tileVertices);
	tile_timers.swap(tile.tile_timers);
	isParallax = tile.isParallax;
	std::swap(parallax, tile.parallax);
	return *this;
}

void TileLayer::initFromAsset(const LayerRef& layerData, bool initCollision) {
	clear();
	assert(layerData.type == LayerType::TILELAYER);

	ref = &layerData;
	hasCollision = initCollision;
	layerID = layerData.id;

	size.x = layerData.tileLayer->internalSize.x == 0 ? layerData.tileLayer->tileSize.x : layerData.tileLayer->internalSize.x;
	size.y = layerData.tileLayer->internalSize.y == 0 ? layerData.tileLayer->tileSize.y : layerData.tileLayer->internalSize.y;
	isParallax = layerData.tileLayer->isParallax;

	if (isParallax) {
		parallax.initOffset.x = (float)(std::min(size.x, GAME_TILE_W) * TILESIZE) / 2.f;
		parallax.initOffset.y = (float)(std::min(size.y, GAME_TILE_H) * TILESIZE) / 2.f;
		parallax.camFactor = Vec2f{1.f, 1.f};
		if (size.x > GAME_TILE_W) {
			parallax.camFactor.x = 1.f - ((float)(size.x - GAME_TILE_W) / (float)layerData.tileLayer->tileSize.x);
		}
		if (size.y > GAME_TILE_H) {
			parallax.camFactor.y = 1.f - ((float)(size.y - GAME_TILE_H) / (float)layerData.tileLayer->tileSize.y);
		}
	}

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

	// update tile timers
	for (auto& timer : tile_timers) {
		timer.time_to_anim -= deltaTime;

		if (timer.time_to_anim < 0.0) {
			changedTile = true;
		}
	}

	// update tiles
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
					tex_pos = Vec2u{ t.next_offset };
				}
				else {
					// if same tileset, use as relative
					tex_pos = timer.tex_position + t.next_offset;
				}
				setTile(timer.tile_impacted, tex_pos, *tileset);

			}
		}
		collision->applyChanges();
	}


	// parallax update
	static secs buff = 0.0;
	buff += deltaTime;
	if (isParallax) {

		Vec2f camPos = m_context.camera()->currentPosition;

		Vec2f pos = Vec2f{
			m_context.camera()->currentPosition.x * parallax.camFactor.x,
			m_context.camera()->currentPosition.y * parallax.camFactor.y
		} - parallax.initOffset;

		for (auto& vta_pair : tileVertices) {
			vta_pair.second.offset = pos;
		}
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

void TileLayer::draw(RenderTarget& target, RenderState states) const {
	//states.transform.translate(offset);

	states.transform = Transform::combine(states.transform, Transform(offset));


	if (!hidden) {
		for (auto& layer : tileVertices) {
			states.texture = layer.first->getTexture();
			target.draw(layer.second, states);
		}
	}
}

}