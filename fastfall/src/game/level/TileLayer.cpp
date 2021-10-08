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
	, layerID(id)
	, level_size(levelsize)
	, tiles((size_t)levelsize.x * levelsize.y)
{
}

TileLayer::TileLayer(GameContext context, unsigned id, const TileLayerData& layerData)
	: m_context(context)
	, layerID(id)
{
	initFromAsset(layerData, id);
}

TileLayer::TileLayer(const TileLayer& tile)
	: m_context(tile.m_context)
	, tiles(tile.tiles)
{
	layerID = tile.layerID;
	level_size = tile.level_size;

	chunks = tile.chunks;

	parallax = tile.parallax;
	scroll = tile.scroll;

	tileLogic.clear();
	for (const auto& logic : tile.tileLogic) {
		tileLogic.push_back(logic->clone(m_context));
	}

	if (tile.collision.enabled) {
		set_collision(tile.collision.enabled, tile.collision.border);
		
		collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}

}
TileLayer& TileLayer::operator=(const TileLayer& tile) {
	m_context = tile.m_context;
	layerID = tile.layerID;
	level_size = tile.level_size;

	chunks = tile.chunks;
	tiles = tile.tiles;

	parallax = tile.parallax;
	scroll = tile.scroll;

	tileLogic.clear();
	for (const auto& logic : tile.tileLogic) {
		tileLogic.push_back(logic->clone(m_context));
	}

	if (tile.collision.enabled) {
		set_collision(tile.collision.enabled, tile.collision.border);

		collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}

	return *this;
}
TileLayer::TileLayer(TileLayer&& tile) noexcept
	: m_context(tile.m_context)
{
	layerID = tile.layerID;
	level_size = tile.level_size;

	std::swap(tiles, tile.tiles);
	std::swap(chunks, tile.chunks);
	std::swap(parallax, tile.parallax);
	std::swap(scroll, tile.scroll);
	std::swap(collision, tile.collision);
	std::swap(tileLogic, tile.tileLogic);

	if (collision.enabled) {
		collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}
}
TileLayer& TileLayer::operator=(TileLayer&& tile) noexcept {
	m_context = tile.m_context;

	layerID = tile.layerID;
	level_size = tile.level_size;

	std::swap(tiles, tile.tiles);
	std::swap(chunks, tile.chunks);
	std::swap(parallax, tile.parallax);
	std::swap(scroll, tile.scroll);
	std::swap(collision, tile.collision);
	std::swap(tileLogic, tile.tileLogic);

	if (collision.enabled) {
		collision.tilemap_ptr->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision.tilemap_ptr->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}
	return *this;
}


TileLayer::~TileLayer() {
	if (m_context.valid() && collision.enabled) {
		instance::phys_erase_collider(m_context, collision.tilemap_ptr);
		collision.tilemap_ptr = nullptr;
	}
}

void TileLayer::initFromAsset(const TileLayerData& layerData, unsigned id) {
	clear();

	auto& tileLayer = layerData;

	layerID = id;

	level_size = layerData.getSize();

	tiles = TileData{ (size_t)level_size.x * level_size.y };

	set_collision(tileLayer.hasCollision(), tileLayer.getCollisionBorders());
	set_parallax(tileLayer.hasParallax(), tileLayer.getParallaxSize());
	set_scroll(tileLayer.hasScrolling(), tileLayer.getScrollRate());

	const auto& tiles = tileLayer.getTiles();
	for (int i = 0; i < tileLayer.getSize().x * tileLayer.getSize().y; i++) {
		if (!tiles.has_tile[i])
			continue;

		const std::string* tileset = tileLayer.getTilesetFromNdx(tiles.tileset_ndx[i]);

		TilesetAsset* ta = Resources::get<TilesetAsset>(*tileset);
		if (ta) {
			setTile(tiles.pos[i], tiles.tex_pos[i], *ta);
		}
		else {
			LOG_ERR_("unknown tileset {}", *tileset);
		}
	}

}

void TileLayer::update(secs deltaTime) {
	if (collision.enabled) {
		collision.tilemap_ptr->update(deltaTime);
	}
	for (auto& logic : tileLogic) {
		logic->update(deltaTime);
	}
}

bool TileLayer::set_collision(bool enabled, unsigned border)
{
	if (enabled && (scroll.enabled || parallax.enabled)) {
		LOG_ERR_("Cannot enable collision on layer with scrolling or parallax");
		return false;
	}

	if (enabled && !collision.enabled) 
	{
		if (m_context.valid() && !collision.tilemap_ptr) {
			collision.tilemap_ptr = instance::phys_create_collider<ColliderTileMap>(m_context, Vec2i(level_size.x, level_size.y), true);
			collision.enabled = true;
			collision.border = border;

			for (unsigned i = 0u; i < tiles.size(); i++) {
				//auto& tile_data = tiles.at(i);

				if (tiles.has_tile[i] && tiles.tileset_id[i] != TILEDATA_NONE)
				{

					const TilesetAsset* tileset = chunks.at(tiles.tileset_id[i]).tileset;

					Tile tile = tileset->getTile(tiles.tex_pos[i]);

					collision.tilemap_ptr->setTile(
						Vec2i{ (int)(i % level_size.x), (int)(i / level_size.x) },
						tile.shape,
						&tileset->getMaterial(tiles.tex_pos[i]),
						tile.matFacing
					);
				}
			}
			collision.tilemap_ptr->setBorders(level_size, collision.border);
			collision.tilemap_ptr->applyChanges();

			collision.tilemap_ptr->set_on_precontact(
				std::bind(&TileLayer::handlePreContact, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
			);
			collision.tilemap_ptr->set_on_postcontact(
				std::bind(&TileLayer::handlePostContact, this,
					std::placeholders::_1, std::placeholders::_2)
			);
		}
	}
	else if (!enabled && collision.enabled)
	{
		if (m_context.valid() && collision.tilemap_ptr) {
			instance::phys_erase_collider(m_context, collision.tilemap_ptr);

		}
		collision.tilemap_ptr = nullptr;
		collision.enabled = false;
	}
	return true;
}


bool TileLayer::set_parallax(bool enabled, Vec2u parallax_size)
{
	if (enabled && collision.enabled) {
		LOG_ERR_("Cannot enable parallax on layer with collision");
		return false;
	}

	bool updateChunks = parallax.enabled != enabled || (parallax.enabled && parallax_size != parallax.size);

	parallax.enabled = enabled;
	parallax.size = parallax_size;
	if (parallax.enabled) {
		parallax.init_offset.x = (float)(std::min(level_size.x, GAME_TILE_W) * TILESIZE) / 2.f;
		parallax.init_offset.y = (float)(std::min(level_size.y, GAME_TILE_H) * TILESIZE) / 2.f;
		parallax.cam_factor = Vec2f{ 1.f, 1.f };
		if (level_size.x > GAME_TILE_W) {
			parallax.cam_factor.x = 1.f - ((float)(parallax.size.x - GAME_TILE_W) / (float)level_size.x);
		}
		if (level_size.y > GAME_TILE_H) {
			parallax.cam_factor.y = 1.f - ((float)(parallax.size.y - GAME_TILE_H) / (float)level_size.y);
		}
	}
	else {
		// reset offset on each chunk
		for (auto& vta_pair : chunks) {
			vta_pair.varray.offset = Vec2f{};
		}
	}

	for (auto& chunk : chunks) {
		chunk.varray.set_size(get_size());
	}
	return true;
}
bool TileLayer::set_scroll(bool enabled, Vec2f rate)
{
	if (enabled && collision.enabled) {
		LOG_ERR_("Cannot enable scrolling on layer with collision");
		return false;
	}

	if (scroll.enabled && !enabled)
	{
		scroll.offset = Vec2f{};
		for (auto& vta_pair : chunks) {
			vta_pair.varray.scroll = Vec2f{};
		}
	}

	scroll.enabled = enabled;
	scroll.rate = rate;
	return true;
}

bool TileLayer::handlePreContact(Vec2i pos, const Contact& contact, secs duration) {
	if (pos.x < 0 || pos.x >= level_size.x
		|| pos.y < 0 || pos.y >= level_size.y)
		return true;

	unsigned ndx = pos.y * level_size.x + pos.x;
	bool r = true;
	if (tiles.logic_id[ndx] != TILEDATA_NONE) {
		r = tileLogic.at(tiles.logic_id[ndx])->on_precontact(pos, contact, duration);
	}
	return r;
}

void TileLayer::handlePostContact(Vec2i pos, const PersistantContact& contact) {
	if (pos.x < 0 || pos.x >= level_size.x
		|| pos.y < 0 || pos.y >= level_size.y)
		return;

	unsigned ndx = pos.y * level_size.x + pos.x;
	if (tiles.logic_id[ndx] != TILEDATA_NONE) {
		tileLogic.at(tiles.logic_id[ndx])->on_postcontact(pos, contact);
	}
}

void TileLayer::predraw(secs deltaTime) {

	bool changed = false;

	// update logic
	for (auto& logic : tileLogic) {
		TileLogic* ptr = logic.get();
		
		while (ptr->hasNextCommand()) {

			const TileLogicCommand& cmd = ptr->nextCommand();

			unsigned ndx = cmd.position.x + level_size.x * cmd.position.y;
			if (cmd.type == TileLogicCommand::Type::Set) {
				setTile(cmd.position, cmd.texposition, cmd.tileset, cmd.updateLogic);
			}
			else if (tiles.has_tile[ndx] && cmd.type == TileLogicCommand::Type::Remove) {
				removeTile(cmd.position);
			}
			changed = true;
			
			ptr->popCommand();
		}
	}

	if (changed && collision.enabled)
		collision.tilemap_ptr->applyChanges();



	Rectf visible;
	Vec2f cam_pos = instance::cam_get_pos(m_context);
	float cam_zoom = instance::cam_get_zoom(m_context);
	visible.width = GAME_W_F * cam_zoom;
	visible.height = GAME_H_F * cam_zoom;
	visible.left = cam_pos.x - (visible.width / 2.f);
	visible.top = cam_pos.y - (visible.height / 2.f);
	for (auto& verts : chunks) {
		verts.varray.visibility = visible;
	}

	// parallax update
	if (parallax.enabled) {

		parallax.offset = Vec2f{ 
			cam_pos.x * parallax.cam_factor.x, 
			cam_pos.y * parallax.cam_factor.y 
		} - parallax.init_offset;

		for (auto& vta_pair : chunks) {
			vta_pair.varray.offset = parallax.offset;
		}
	}

	// scroll update
	if (scroll.enabled) {
		Vec2f scroll_delta = scroll.rate * deltaTime;


		scroll.offset += scroll_delta;

		Vec2f sizef = Vec2f{ get_size() } * TILESIZE_F;

		while (scroll.offset.x < 0.f) scroll.offset.x += sizef.x;
		while (scroll.offset.x >= sizef.x) scroll.offset.x -= sizef.x;

		while (scroll.offset.y < 0.f) scroll.offset.y += sizef.y;
		while (scroll.offset.y >= sizef.y) scroll.offset.y -= sizef.y;


		for (auto& vta_pair : chunks) {
			vta_pair.varray.scroll = scroll.offset;
		}
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::TILELAYER_AREA))
	{
		Vec2f pSize = Vec2f{ level_size } * TILESIZE_F;

		size_t ptr = (size_t)this;
		if (!debug_draw::repeat((void*)(ptr), parallax.offset)) {
			debug_draw::set_offset(parallax.offset);
			auto& drawable1 = createDebugDrawable<ShapeRectangle, debug_draw::Type::TILELAYER_AREA>(
				(const void*)(ptr), Rectf({ 0, 0 }, pSize), Color::Transparent, Color::Red
			);
			debug_draw::set_offset();
		}
		if (!debug_draw::repeat((void*)(ptr + 1), parallax.offset + scroll.offset)) {
			debug_draw::set_offset(parallax.offset + scroll.offset);
			auto& drawable2 = createDebugDrawable<ShapeRectangle, debug_draw::Type::TILELAYER_AREA>(
				(const void*)(ptr + 1), Rectf({ 0, 0 }, pSize), Color::Transparent, Color::Green
			);
			debug_draw::set_offset();
		}
	}

	if (!hidden) {
		for (auto& vta_pair : chunks) {
			vta_pair.varray.predraw();
		}
	}

}

void TileLayer::setTile(const Vec2u& position, const Vec2u& texposition, const TilesetAsset& tileset, bool useLogic) {

	// blank existing tile at that position
	unsigned ndx = position.y * level_size.x + position.x;
	uint8_t tileset_ndx = tiles.tileset_id[ndx];
	uint8_t logic_ndx = tiles.logic_id[ndx];

	tiles.tex_pos[ndx] = texposition;
	tiles.has_tile[ndx] = true;

	if (tileset_ndx != TILEDATA_NONE) {
		chunks.at(tileset_ndx).varray.blank(position);
		tiles.tileset_id[ndx] = TILEDATA_NONE;
	}
	if (useLogic && logic_ndx != TILEDATA_NONE) {
		tileLogic.at(logic_ndx)->removeTile(position);
		tiles.logic_id[ndx] = TILEDATA_NONE;
	}

	// find tilearray or create it
	auto vertarr = std::find_if(chunks.begin(), chunks.end(), [&tileset](const ChunkVA& tva) {
		return tva.tileset == &tileset;
		});
	if (vertarr == chunks.end()) {

		if (chunks.size() < UINT8_MAX - 1) {

			chunks.push_back(ChunkVA{
					.tileset = &tileset,
					.varray = ChunkVertexArray(
							get_size(),
							kChunkSize
						)
				});

			chunks.back().varray.setTexture(tileset.getTexture());
			chunks.back().varray.setTile(position, texposition);
			chunks.back().varray.use_visible_rect = true;
			tiles.tileset_id[ndx] = chunks.size() - 1;
		}
		else {
			LOG_ERR_("Cannot set tile, tilelayer has reached max tileset references: {}", chunks.size());
		}
	}
	else {
		vertarr->varray.setTile(position, texposition);
		tiles.tileset_id[ndx] = std::distance(chunks.begin(), vertarr);
	}

	if (collision.enabled) {
		Tile t = tileset.getTile(texposition);
		collision.tilemap_ptr->setTile(
			Vec2i(position), 
			t.shape, 
			&tileset.getMaterial(texposition), 
			t.matFacing
		);
	}

	if (useLogic) {

		if (auto [logic, args] = tileset.getTileLogic(texposition); !logic.empty()) {
			std::string_view logic_var = logic;

			auto it = std::find_if(tileLogic.begin(), tileLogic.end(), 
				[logic_var](const std::unique_ptr<TileLogic>& log) {
					return logic_var == log->getName();
				});

			Tile t = tileset.getTile(texposition);

			if (it == tileLogic.end()) {
				if (tileLogic.size() < UINT8_MAX - 1) {

					auto logic_ptr = TileLogic::create(m_context, logic);
					if (logic_ptr) {
						tileLogic.push_back(std::move(logic_ptr));
						tileLogic.back()->addTile(position, t, args.data());
						tiles.logic_id[ndx] = tileLogic.size() - 1;
					}
					else {
						LOG_WARN("could not create tile logic type: {}", logic);
					}
				}
				else {
					LOG_ERR_("Cannot set tile logic, tilelayer has reached max logic references: {}", tileLogic.size());
				}
			}
			else {
				it->get()->addTile(position, t, args.data());
				tiles.logic_id[ndx] = std::distance(tileLogic.begin(), it);
			}
		}
	}
}

void TileLayer::removeTile(const Vec2u& position) {
	unsigned ndx = position.y * level_size.x + position.x;

	if (tiles.tileset_id[ndx] != TILEDATA_NONE) {
		chunks.at(tiles.tileset_id[ndx]).varray.blank(position);
	}
	if (tiles.logic_id[ndx] != TILEDATA_NONE) {
		tileLogic.at(tiles.logic_id[ndx])->removeTile(position);
	}
	tiles.clear(ndx);
	
	if (collision.enabled) {
		collision.tilemap_ptr->removeTile(Vec2i(position));
	}
}

void TileLayer::clear() {
	//pos2data.clear();
	tiles = TileData{};
	chunks.clear();
	set_collision(false);
	set_parallax(false);
	set_scroll(false);
}

void TileLayer::shallow_copy(const TileLayer& layer, Rectu area, Vec2u lvlSize)
{
	for (unsigned y = area.top; y < area.top + area.height; y++) {
		for (unsigned x = area.left; x < area.left + area.width; x++) {
			if (layer.level_size.x <= x || layer.level_size.y <= y)
				continue;

			Vec2u pos{ x,y };
			unsigned ndx = pos.y * layer.level_size.x + pos.x;
			if (!layer.tiles.has_tile[ndx])
				continue;

			const TilesetAsset* tileset = nullptr;

			Vec2u tex_pos = layer.tiles.tex_pos[ndx];

			if (layer.tiles.tileset_id[ndx] != UINT8_MAX) {
				tileset = layer.chunks.at(layer.tiles.tileset_id[ndx]).tileset;
			}
			if (!tileset)
				continue;

			setTile(pos, tex_pos, *tileset);
		}
	}
}

bool TileLayer::hasTileAt(Vec2u tile_pos)
{
	unsigned ndx = tile_pos.y * level_size.x + tile_pos.x;
	if (tile_pos.x < level_size.x && tile_pos.y < level_size.y)
	{
		return tiles.has_tile[ndx];
	}
	return false;
}

std::optional<Vec2u> TileLayer::getTileTexPos(Vec2u tile_pos)
{
	unsigned ndx = tile_pos.y * level_size.x + tile_pos.x;
	if (hasTileAt(tile_pos))
	{
		return tiles.tex_pos[ndx];
	}
	return std::nullopt;
}

const TilesetAsset* TileLayer::getTileTileset(Vec2u tile_pos)
{
	unsigned ndx = tile_pos.y * level_size.x + tile_pos.x;
	if (hasTileAt(tile_pos) && tiles.tileset_id[ndx] != TILEDATA_NONE)
	{
		return chunks.at(tiles.tileset_id[ndx]).tileset;
	}
	return nullptr;
}


Vec2f TileLayer::getWorldPosFromTilePos(Vec2i tile_pos) const {


	Vec2f pos = Vec2f{ tile_pos * TILESIZE };
	pos += get_total_offset();

	if (pos.x >= get_size().x * TILESIZE_F) pos.x -= get_size().x * TILESIZE_F;
	if (pos.y >= get_size().y * TILESIZE_F) pos.y -= get_size().y * TILESIZE_F;

	return pos;
}

std::optional<Vec2i> TileLayer::getTileFromWorldPos(Vec2f position) const {
	position -= get_total_offset();

	position.x = floorf(position.x / TILESIZE_F);
	position.y = floorf(position.y / TILESIZE_F);

	Vec2i tile_pos = Vec2i{ position };

	if (tile_pos.x < 0) tile_pos.x += get_size().x;
	if (tile_pos.y < 0) tile_pos.y += get_size().y;

	return tile_pos;

}

void TileLayer::draw(RenderTarget& target, RenderState states) const {
	states.transform = Transform::combine(states.transform, Transform(offset));

	if (!hidden) {
		for (auto& layer : chunks) {
			states.texture = layer.varray.getTexture();

			target.draw(layer.varray, states);
		}
	}
}

}
