#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/RenderTarget.hpp"

#include "fastfall/game/GameCamera.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/ShapeRectangle.hpp"

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
	has_parallax = tile.has_parallax;
	parallax = tile.parallax;
	scrollRate = tile.scrollRate;
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
	has_parallax = tile.has_parallax;
	parallax = tile.parallax;
	scrollRate = tile.scrollRate;
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
	has_parallax = tile.has_parallax;
	std::swap(parallax, tile.parallax);
	std::swap(scrollRate, tile.scrollRate);
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
	has_parallax = tile.has_parallax;
	std::swap(parallax, tile.parallax);
	std::swap(scrollRate, tile.scrollRate);
	return *this;
}

void TileLayer::initFromAsset(const LayerRef& layerData, bool initCollision) {
	clear();
	assert(layerData.type == LayerType::TILELAYER);

	ref = &layerData;
	hasCollision = initCollision;
	layerID = layerData.id;

	size.x = layerData.tileLayer->innerSize.x == 0 ? layerData.tileLayer->tileSize.x : layerData.tileLayer->innerSize.x;
	size.y = layerData.tileLayer->innerSize.y == 0 ? layerData.tileLayer->tileSize.y : layerData.tileLayer->innerSize.y;

	has_parallax = layerData.tileLayer->has_parallax;
	scrollRate = layerData.tileLayer->scrollrate;

	/*
	if (hasScrollX()) {
		size.x++;
	}
	if (hasScrollY()) {
		size.y++;
	}
	*/

	// calc parallax factors
	if (has_parallax) {
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

			/*
			if (hasScrollX() && hasScrollY()
				&& i.first.x == 0 && i.first.y == 0)
			{
				setTile(Vec2u(size.x - 1, i.first.y),  i.second.texPos, *ta);
				setTile(Vec2u(i.first.x, size.y - 1),  i.second.texPos, *ta);
				setTile(Vec2u(size.x - 1, size.y - 1), i.second.texPos, *ta);
			}
			else if (hasScrollX() && i.first.x == 0) {
				setTile(Vec2u(size.x - 1, i.first.y),  i.second.texPos, *ta);
			}
			else if (hasScrollY() && i.first.y == 0) {
				setTile(Vec2u(i.first.x, size.y - 1),  i.second.texPos, *ta);
			}
			*/

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
		if (hasCollision)
			collision->applyChanges();
	}


	// parallax update
	static secs buff = 0.0;
	buff += deltaTime;
	Vec2f parallax_offset;
	if (has_parallax) {
		parallax_offset = Vec2f{
			m_context.camera()->currentPosition.x * parallax.camFactor.x,
			m_context.camera()->currentPosition.y * parallax.camFactor.y
		} - parallax.initOffset;
	}

	// scroll update
	Vec2f pSize = Vec2f{ size } * TILESIZE_F;
	if (hasScrollX() || hasScrollY()) {
		scroll_offset += scrollRate * deltaTime;

		while (scroll_offset.x < 0.f) {	
			scroll_offset.x += TILESIZE_F;
			for (auto& vta_pair : tileVertices)	vta_pair.second.rotate_backwardX();
		}
		while (scroll_offset.x >= TILESIZE_F) {
			scroll_offset.x -= TILESIZE_F;
			for (auto& vta_pair : tileVertices)	vta_pair.second.rotate_forwardX();
		}
		while (scroll_offset.y < 0.f) { 
			scroll_offset.y += TILESIZE_F;
			for (auto& vta_pair : tileVertices)	vta_pair.second.rotate_backwardY();
		}
		while (scroll_offset.y >= TILESIZE_F) {
			scroll_offset.y -= TILESIZE_F;
			for (auto& vta_pair : tileVertices)	vta_pair.second.rotate_forwardY();
		}

	}

	if (has_parallax || hasScrollX() || hasScrollY()) {
		for (auto& vta_pair : tileVertices) {
			vta_pair.second.offset = parallax_offset + scroll_offset;
		}
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::TILELAYER_AREA))
	{
		size_t ptr = (size_t)this;
		if (!debug_draw::repeat((void*)(ptr), parallax_offset)) {
			debug_draw::set_offset(parallax_offset);
			auto& drawable1 = createDebugDrawable<ShapeRectangle>((const void*)(ptr), Rectf({ 0, 0 }, pSize), Color::Transparent, Color::Red);
			debug_draw::set_offset();
		}
		if (!debug_draw::repeat((void*)(ptr + 1), parallax_offset + scroll_offset)) {
			debug_draw::set_offset(parallax_offset + scroll_offset);
			auto& drawable2 = createDebugDrawable<ShapeRectangle>((const void*)(ptr + 1), Rectf({ 0, 0 }, pSize), Color::Transparent, Color::Green);
			debug_draw::set_offset();
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
			/*
			if (hasScrollX() && hasScrollY()) {
				RenderState state_off = states;
				Vec2f off = Vec2f{ size } * TILESIZE_F;
				state_off.transform = Transform::combine(states.transform, Transform(-off));
				target.draw(layer.second, state_off);


			}
			if (hasScrollX()) {
				RenderState state_off = states;
				Vec2f off = Vec2f{ (float)size.x , 0.f } * TILESIZE_F;
				state_off.transform = Transform::combine(states.transform, Transform(-off));
				target.draw(layer.second, state_off);

			}
			if (hasScrollY()) {
				RenderState state_off = states;
				Vec2f off = Vec2f{ 0.f, (float)size.y } * TILESIZE_F;
				state_off.transform = Transform::combine(states.transform, Transform(-off));
				target.draw(layer.second, state_off);

			}
			*/
		}
	}
}

}