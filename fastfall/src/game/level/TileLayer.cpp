#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/game/level/ObjectLayer.hpp"
#include "fastfall/resource/Resources.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/RenderTarget.hpp"

#include "fastfall/game/GameCamera.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/game/CollisionManager.hpp"

#include "fastfall/game/InstanceInterface.hpp"

#include <assert.h>

namespace ff {


TileLayer::TileLayer(GameContext context, unsigned id, Vec2u levelsize)
	: m_context(context)
	, layer_data(id, levelsize)
	, tiles_dyn(levelsize)
{
}

TileLayer::TileLayer(GameContext context, const TileLayerData& layerData)
	: m_context(context)
{
	initFromAsset(layerData);
}

TileLayer::TileLayer(const TileLayer& tile)
	: m_context(tile.m_context)
	, layer_data(tile.layer_data)
	, tiles_dyn(tile.tiles_dyn)
{

	dyn.chunks = tile.dyn.chunks;
	dyn.parallax = tile.dyn.parallax;
	dyn.scroll = tile.dyn.scroll;

	dyn.tile_logic.clear();
	for (const auto& logic : tile.dyn.tile_logic) {
		dyn.tile_logic.push_back(logic->clone(m_context));
	}

	set_collision(false);
	if (tile.hasCollision()) {
		set_collision(tile.hasCollision(), tile.getCollisionBorders());

		dyn.collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		dyn.collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}
}
TileLayer& TileLayer::operator=(const TileLayer& tile) {

	m_context = tile.m_context;
	layer_data = tile.layer_data;
	tiles_dyn = tile.tiles_dyn;

	dyn.chunks = tile.dyn.chunks;
	dyn.parallax = tile.dyn.parallax;
	dyn.scroll = tile.dyn.scroll;

	dyn.tile_logic.clear();
	for (const auto& logic : tile.dyn.tile_logic) {
		dyn.tile_logic.push_back(logic->clone(m_context));
	}

	set_collision(false);
	if (tile.hasCollision()) {
		set_collision(tile.hasCollision(), tile.getCollisionBorders());

		dyn.collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		dyn.collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}

	return *this;
}
TileLayer::TileLayer(TileLayer&& tile) noexcept
	: m_context(tile.m_context)
{

	std::swap(layer_data, tile.layer_data);
	std::swap(tiles_dyn, tile.tiles_dyn);

	std::swap(dyn.chunks, tile.dyn.chunks);
	std::swap(dyn.parallax, tile.dyn.parallax);
	std::swap(dyn.scroll, tile.dyn.scroll);

	std::swap(dyn.collision, tile.dyn.collision);
	std::swap(dyn.tile_logic, tile.dyn.tile_logic);

	if (hasCollision()) {
		dyn.collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		dyn.collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}
}
TileLayer& TileLayer::operator=(TileLayer&& tile) noexcept {
	m_context = tile.m_context;

	std::swap(layer_data, tile.layer_data);
	std::swap(tiles_dyn, tile.tiles_dyn);

	std::swap(dyn.chunks, tile.dyn.chunks);
	std::swap(dyn.parallax, tile.dyn.parallax);
	std::swap(dyn.scroll, tile.dyn.scroll);

	std::swap(dyn.collision, tile.dyn.collision);
	std::swap(dyn.tile_logic, tile.dyn.tile_logic);

	if (hasCollision()) {
		dyn.collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		dyn.collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}
	return *this;
}


TileLayer::~TileLayer() {
	if (m_context.valid() && dyn.collision.tilemap_ptr) {
		instance::phys_erase_collider(m_context, dyn.collision.tilemap_ptr);
		dyn.collision.tilemap_ptr = nullptr;
	}
}

void TileLayer::initFromAsset(const TileLayerData& layerData) {
	clear();

	layer_data = layerData;

	// init chunks
	for (auto& [tileset, _] : layer_data.getTilesets())
	{
		dyn.chunks.push_back(ChunkVertexArray(getSize(), kChunkSize));
		dyn.chunks.back().setTexture(tileset->getTexture());
		dyn.chunks.back().use_visible_rect = true;
	}

	// init tiles
	const auto& tiles = layer_data.getTileData();
	unsigned tile_count = layer_data.getSize().x * layer_data.getSize().y;

	tiles_dyn = grid_vector<TileDynamic>(layer_data.getSize());

	for (const auto& tile : tiles)
	{
		if (!tile.has_tile)
			continue;

		const auto* tileset = layer_data.getTilesetFromNdx(tile.tileset_ndx);
		auto& chunk = dyn.chunks.at(tile.tileset_ndx);

		chunk.setTile(tile.pos, tile.tile_id);

		if (auto [logic, args] = tileset->getTileLogic(tile.tile_id); !logic.empty())
		{
			uint8_t logic_ndx = 0;
			auto logic_it = std::find_if(dyn.tile_logic.begin(), dyn.tile_logic.end(),
				[l = logic, &logic_ndx](const std::unique_ptr<TileLogic>& ptr) {
					logic_ndx++;
					return ptr->getName() == l;
				});

			auto& dyn_tile = tiles_dyn[tile.pos];

			if (logic_it != dyn.tile_logic.end())
			{
				dyn_tile.logic_id = logic_ndx - 1;
				logic_it->get()->addTile(tile.pos, tileset->getTile(tile.tile_id), args);
			}
			else
			{
				dyn_tile.logic_id = dyn.tile_logic.size();
				dyn.tile_logic.push_back(TileLogic::create(m_context, logic));
				dyn.tile_logic.back()->addTile(tile.pos, tileset->getTile(tile.tile_id), args);
			}
		}
	}


	set_collision(layerData.hasCollision(), layerData.getCollisionBorders());
	set_parallax(layerData.hasParallax(), layerData.getParallaxSize());
	set_scroll(layerData.hasScrolling(), layerData.getScrollRate());

	for (auto& chunk : dyn.chunks) {
		chunk.predraw();
	}

}

void TileLayer::update(secs deltaTime) {
	if (hasCollision()) {
		dyn.collision.tilemap_ptr->update(deltaTime);
	}
	for (auto& logic : dyn.tile_logic) {
		logic->update(deltaTime);
	}
}

bool TileLayer::set_collision(bool enabled, unsigned border)
{
	if (enabled && (hasScrolling() || hasParallax())) {
		LOG_ERR_("Cannot enable collision on layer with scrolling or parallax");
		return false;
	}

	if (enabled)
	{
		if (m_context.valid() && !dyn.collision.tilemap_ptr) {

			dyn.collision.tilemap_ptr = instance::phys_create_collider<ColliderTileMap>(m_context, Vec2i{ getLevelSize() }, true);
			layer_data.setCollision(true, border);

			for (const auto& tile_data : layer_data.getTileData())
			{
				if (tile_data.has_tile && tile_data.tileset_ndx != TILEDATA_NONE)
				{
					const TilesetAsset* tileset = layer_data.getTilesetFromNdx(tile_data.tileset_ndx);

					Tile tile = tileset->getTile(tile_data.tile_id);

					dyn.collision.tilemap_ptr->setTile(
						Vec2i{ tile_data.pos },
						tile.shape,
						&tileset->getMaterial(tile_data.tile_id),
						tile.matFacing
					);
				}
			}

			dyn.collision.tilemap_ptr->setBorders(getLevelSize(), layer_data.getCollisionBorders());
			dyn.collision.tilemap_ptr->applyChanges();

			dyn.collision.tilemap_ptr->set_on_precontact(
				std::bind(&TileLayer::handlePreContact, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
			);
			dyn.collision.tilemap_ptr->set_on_postcontact(
				std::bind(&TileLayer::handlePostContact, this,
					std::placeholders::_1, std::placeholders::_2)
			);
		}
	}
	else if (!enabled)
	{
		if (m_context.valid() && dyn.collision.tilemap_ptr) {
			instance::phys_erase_collider(m_context, dyn.collision.tilemap_ptr);
		}
		dyn.collision.tilemap_ptr = nullptr;
		layer_data.setCollision(false);
	}
	return true;
}


bool TileLayer::set_parallax(bool enabled, Vec2u parallax_size)
{
	if (enabled && hasCollision()) {
		LOG_ERR_("Cannot enable parallax on layer with collision");
		return false;
	}

	//bool updateChunks = parallax.enabled != enabled || (parallax.enabled && parallax_size != parallax.size);

	layer_data.setParallax(enabled, parallax_size);
	if (hasParallax()) {
		dyn.parallax.init_offset.x = (float)(std::min(getLevelSize().x, GAME_TILE_W) * TILESIZE) / 2.f;
		dyn.parallax.init_offset.y = (float)(std::min(getLevelSize().y, GAME_TILE_H) * TILESIZE) / 2.f;
		dyn.parallax.cam_factor = Vec2f{ 1.f, 1.f };
		if (getLevelSize().x > GAME_TILE_W) {
			dyn.parallax.cam_factor.x = 1.f - ((float)(getParallaxSize().x - GAME_TILE_W) / (float)getLevelSize().x);
		}
		if (getLevelSize().y > GAME_TILE_H) {
			dyn.parallax.cam_factor.y = 1.f - ((float)(getParallaxSize().y - GAME_TILE_H) / (float)getLevelSize().y);
		}
	}
	else {
		// reset offset on each chunk
		for (auto& chunk : dyn.chunks) {
			chunk.offset = Vec2f{};
		}
	}

	for (auto& chunk : dyn.chunks) {
		chunk.set_size(getSize());
	}
	return true;
}
bool TileLayer::set_scroll(bool enabled, Vec2f rate)
{
	if (enabled && hasCollision()) {
		LOG_ERR_("Cannot enable scrolling on layer with collision");
		return false;
	}

	if (layer_data.hasScrolling() && !enabled)
	{
		dyn.scroll.offset = Vec2f{};
		for (auto& chunk : dyn.chunks) {
			chunk.scroll = Vec2f{};
		}
	}

	layer_data.setScroll(enabled, rate);
	return true;
}

bool TileLayer::handlePreContact(Vec2i pos, const Contact& contact, secs duration) {
	if (pos.x < 0 || pos.x >= getLevelSize().x
		|| pos.y < 0 || pos.y >= getLevelSize().y)
		return true;

	//unsigned ndx = pos.y * getLevelSize().x + pos.x;
	bool r = true;
	if (tiles_dyn[pos].logic_id != TILEDATA_NONE) {
		r = dyn.tile_logic.at(tiles_dyn[pos].logic_id)->on_precontact(pos, contact, duration);
	}
	return r;
}

void TileLayer::handlePostContact(Vec2i pos, const PersistantContact& contact) {
	if (pos.x < 0 || pos.x >= getLevelSize().x
		|| pos.y < 0 || pos.y >= getLevelSize().y)
		return;

	//unsigned ndx = pos.y * getLevelSize().x + pos.x;
	if (tiles_dyn[pos].logic_id != TILEDATA_NONE) {
		dyn.tile_logic.at(tiles_dyn[pos].logic_id)->on_postcontact(pos, contact);
	}
}

void TileLayer::predraw(secs deltaTime) {
	const auto& tile_data = layer_data.getTileData();

	bool changed = false;

	// update logic
	for (auto& logic : dyn.tile_logic) {
		TileLogic* ptr = logic.get();
		
		while (ptr->hasNextCommand()) {

			const TileLogicCommand& cmd = ptr->nextCommand();

			if (cmd.type == TileLogicCommand::Type::Set) {
				setTile(cmd.position, cmd.texposition, cmd.tileset, cmd.updateLogic);
			}
			else if (tile_data[cmd.position].has_tile && cmd.type == TileLogicCommand::Type::Remove) {
				removeTile(cmd.position);
			}
			changed = true;
			
			ptr->popCommand();
		}
	}

	if (changed && hasCollision())
		dyn.collision.tilemap_ptr->applyChanges();



	Rectf visible;
	Vec2f cam_pos = instance::cam_get_pos(m_context);
	float cam_zoom = instance::cam_get_zoom(m_context);
	visible.width = GAME_W_F * cam_zoom;
	visible.height = GAME_H_F * cam_zoom;
	visible.left = cam_pos.x - (visible.width / 2.f);
	visible.top = cam_pos.y - (visible.height / 2.f);
	for (auto& chunk : dyn.chunks) {
		chunk.visibility = visible;
	}

	// parallax update
	if (hasParallax()) {

		dyn.parallax.offset = Vec2f{ 
			cam_pos.x * dyn.parallax.cam_factor.x,
			cam_pos.y * dyn.parallax.cam_factor.y
		} - dyn.parallax.init_offset;

		for (auto& chunk : dyn.chunks) {
			chunk.offset = dyn.parallax.offset;
		}
	}

	// scroll update
	if (hasScrolling()) {
		dyn.scroll.offset += getScrollRate() * deltaTime;

		Vec2f sizef = Vec2f{ getSize() } * TILESIZE_F;

		while (dyn.scroll.offset.x < 0.f) dyn.scroll.offset.x += sizef.x;
		while (dyn.scroll.offset.x >= sizef.x) dyn.scroll.offset.x -= sizef.x;

		while (dyn.scroll.offset.y < 0.f) dyn.scroll.offset.y += sizef.y;
		while (dyn.scroll.offset.y >= sizef.y) dyn.scroll.offset.y -= sizef.y;


		for (auto& chunk : dyn.chunks) {
			chunk.scroll = dyn.scroll.offset;
		}
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::TILELAYER_AREA))
	{
		Vec2f pSize = Vec2f{ getLevelSize() } * TILESIZE_F;

		size_t ptr = (size_t)this;
		if (!debug_draw::repeat((void*)(ptr), get_parallax_offset())) {
			debug_draw::set_offset(get_parallax_offset());
			auto& drawable1 = createDebugDrawable<ShapeRectangle, debug_draw::Type::TILELAYER_AREA>(
				(const void*)(ptr), Rectf({ 0, 0 }, pSize), Color::Transparent, Color::Red
			);
			debug_draw::set_offset();
		}
		if (!debug_draw::repeat((void*)(ptr + 1), get_total_offset())) {
			debug_draw::set_offset(get_total_offset());
			auto& drawable2 = createDebugDrawable<ShapeRectangle, debug_draw::Type::TILELAYER_AREA>(
				(const void*)(ptr + 1), Rectf({ 0, 0 }, pSize), Color::Transparent, Color::Green
			);
			debug_draw::set_offset();
		}
	}

	if (!hidden) {
		for (auto& chunk : dyn.chunks) {
			chunk.predraw();
		}
	}

}

void TileLayer::setTile(const Vec2u& position, TileID tile_id, const TilesetAsset& tileset, bool useLogic)
{

	auto& tile_data = layer_data.getTileData();
	auto prev_tileset_ndx = tile_data[position].tileset_ndx;

	auto changes = layer_data.setTile(position, tile_id, tileset);

	for (int i = 0; i < changes.count; i++) {
		auto& change = changes.arr[i];
		auto next_tileset_ndx = tile_data[change.position].tileset_ndx;

		updateTile(
			change.position, 
			change.position == position ? prev_tileset_ndx : tile_data[change.position].tileset_ndx, 
			*change.tileset, 
			useLogic);
	}
}



void TileLayer::removeTile(const Vec2u& position) {
	const auto& tile_data = layer_data.getTileData();

	if (tile_data[position].tileset_ndx != TILEDATA_NONE) {
		uint8_t t_ndx = tile_data[position].tileset_ndx;
		dyn.chunks.at(t_ndx).blank(position);
	}
	if (tiles_dyn[position].logic_id != TILEDATA_NONE) {
		dyn.tile_logic.at(tiles_dyn[position].logic_id)->removeTile(position);
	}
	if (hasCollision()) {
		dyn.collision.tilemap_ptr->removeTile(Vec2i(position));
	}

	uint8_t tileset_ndx = tile_data[position].tileset_ndx;
	auto result = layer_data.removeTile(position);
	if (result.erased_tile && result.tileset_remaining == 0) {
		dyn.chunks.erase(dyn.chunks.begin() + tileset_ndx);
	}

	for (int i = 0; i < result.changes.count; i++) {
		auto& change = result.changes.arr[i];
		auto next_tileset_ndx = tile_data[change.position].tileset_ndx;

		updateTile(
			change.position,
			tile_data[change.position].tileset_ndx,
			*change.tileset,
			true);
	}

}

void TileLayer::updateTile(const Vec2u& at, uint8_t prev_tileset_ndx, const TilesetAsset& next_tileset, bool useLogic)
{
	uint8_t tileset_ndx = prev_tileset_ndx;
	if (tileset_ndx != TILEDATA_NONE) {
		dyn.chunks.at(tileset_ndx).blank(at);
	}

	uint8_t logic_ndx = tiles_dyn[at].logic_id;
	if (useLogic && logic_ndx != TILEDATA_NONE) {
		dyn.tile_logic.at(logic_ndx)->removeTile(at);
		tiles_dyn[at].logic_id = TILEDATA_NONE;
	}

	auto& tile_data = layer_data.getTileData();

	if (tile_data[at].tileset_ndx == dyn.chunks.size())
	{

		dyn.chunks.push_back(ChunkVertexArray(getSize(), kChunkSize));
		dyn.chunks.back().setTexture(next_tileset.getTexture());
		dyn.chunks.back().setTile(at, tile_data[at].tile_id);
		dyn.chunks.back().use_visible_rect = true;
	}
	else {
		dyn.chunks.at(tile_data[at].tileset_ndx).setTile(at, tile_data[at].tile_id);
	}

	if (hasCollision()) {
		Tile t = next_tileset.getTile(tile_data[at].tile_id);
		dyn.collision.tilemap_ptr->setTile(
			Vec2i(at),
			t.shape,
			&next_tileset.getMaterial(tile_data[at].tile_id),
			t.matFacing
		);
	}

	if (useLogic) {
		if (auto [logic, args] = next_tileset.getTileLogic(tile_data[at].tile_id); !logic.empty()) {
			std::string_view logic_var = logic;


			unsigned dist = 0;
			auto it = std::find_if(dyn.tile_logic.begin(), dyn.tile_logic.end(),
				[logic_var, &dist](const std::unique_ptr<TileLogic>& log) {
					dist++;
					return logic_var == log->getName();
				});
			dist--;

			Tile t = next_tileset.getTile(tile_data[at].tile_id);

			if (it == dyn.tile_logic.end()) {
				if (dyn.tile_logic.size() < UINT8_MAX - 1) {

					auto logic_ptr = TileLogic::create(m_context, logic);
					if (logic_ptr) {
						dyn.tile_logic.push_back(std::move(logic_ptr));
						dyn.tile_logic.back()->addTile(at, t, args.data());
						tiles_dyn[at].logic_id = dyn.tile_logic.size() - 1;
					}
					else {
						LOG_WARN("could not create tile logic type: {}", logic);
					}
				}
				else {
					LOG_ERR_("Cannot set tile logic, tilelayer has reached max logic references: {}", dyn.tile_logic.size());
				}
			}
			else {
				it->get()->addTile(at, t, args.data());
				tiles_dyn[at].logic_id = dist;
			}
		}
	}
}

void TileLayer::clear() {
	layer_data.clearTiles();
	dyn.chunks.clear();
	dyn.tile_logic.clear();

	tiles_dyn = grid_vector<TileDynamic>(layer_data.getSize());

	set_collision(false);
	set_parallax(false);
	set_scroll(false);
}

void TileLayer::shallow_copy(const TileLayer& src, Rectu src_area, Vec2u dst)
{
	Rectu dst_area{ dst, Vec2u{ src_area.getSize() } };

	Rectu src_level_area{ {}, src.getLevelSize() };
	Rectu dst_level_area{ {}, getLevelSize() };

	src_level_area.intersects(src_area, src_area);
	dst_level_area.intersects(dst_area, dst_area);

	const auto& src_tiles = src.layer_data.getTileData();

	for (const auto& tile : src_tiles.take_view(src_area.getPosition(), src_area.getSize()))
	{
		if (!tile.has_tile)
			continue;

		const TilesetAsset* tileset = nullptr;
		if (tile.tileset_ndx != UINT8_MAX) {
			tileset = src.layer_data.getTilesetFromNdx(tile.tileset_ndx);
		}
		else {
			continue;
		}

		TileID tex_pos = tile.tile_id;
		setTile(tile.pos - dst, tex_pos, *tileset);
	}
}


bool TileLayer::hasTileAt(Vec2u tile_pos)
{
	if (tile_pos.x < getLevelSize().x && tile_pos.y < getLevelSize().y)
	{
		return layer_data.getTileData()[tile_pos].has_tile;
	}
	return false;
}

std::optional<TileID> TileLayer::getTileID(Vec2u tile_pos)
{
	if (hasTileAt(tile_pos))
	{
		return layer_data.getTileData()[tile_pos].tile_id;
	}
	return std::nullopt;
}

const TilesetAsset* TileLayer::getTileTileset(Vec2u tile_pos)
{
	if (hasTileAt(tile_pos))
	{
		return layer_data.getTilesetFromNdx(layer_data.getTileData()[tile_pos].tileset_ndx);
	}
	return nullptr;
}


TileLayer::world_pos_t TileLayer::getWorldPosFromTilePos(Vec2i tile_pos) const {

	Vec2f pos = Vec2f{ tile_pos * TILESIZE } + get_total_offset();

	return world_pos_t{
		.mirrorx = pos.x >= getSize().x * TILESIZE_F,
		.mirrory = pos.y >= getSize().y * TILESIZE_F,
		.real = pos
	};;
}

std::optional<Vec2i> TileLayer::getTileFromWorldPos(Vec2f position) const {
	position -= get_total_offset();

	position.x = floorf(position.x / TILESIZE_F);
	position.y = floorf(position.y / TILESIZE_F);

	Vec2i tile_pos = Vec2i{ position };

	if (tile_pos.x < 0) tile_pos.x += getSize().x;
	if (tile_pos.y < 0) tile_pos.y += getSize().y;

	return tile_pos;

}

void TileLayer::draw(RenderTarget& target, RenderState states) const {
	states.transform = Transform::combine(states.transform, Transform(offset));

	if (!hidden) {
		for (auto& layer : dyn.chunks) {
			states.texture = layer.getTexture();

			target.draw(layer, states);
		}
	}
}

}
