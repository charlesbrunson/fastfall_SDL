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


TileLayer::TileLayer(GameContext context, unsigned id, Vec2u size)
	: m_context(context)
	, layerID(id)
	, size(size)
{
	pos2data.resize((size_t)size.x * size.y, TileData{});
}

TileLayer::TileLayer(GameContext context, unsigned id, const TileLayerRef& layerData)
	: m_context(context)
	, layerID(id)
{
	initFromAsset(layerData, id);
}

TileLayer::TileLayer(const TileLayer& tile)
	: m_context(tile.m_context) 
{
	layerID = tile.layerID;
	size = tile.size;

	chunks = tile.chunks;
	pos2data = tile.pos2data;

	hasParallax = tile.hasParallax;
	parallax = tile.parallax;

	hasScroll = tile.hasScroll;
	scrollRate = tile.scrollRate;

	tileLogic.clear();
	for (const auto& logic : tile.tileLogic) {
		tileLogic.push_back(TileLogic::create(m_context, logic->getName()));
	}

	if (tile.hasCollision) {
		set_collision(tile.hasCollision, tile.collision_border);
		
		collision->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}

}
TileLayer& TileLayer::operator=(const TileLayer& tile) {
	m_context = tile.m_context;
	layerID = tile.layerID;
	size = tile.size;

	chunks = tile.chunks;
	pos2data = tile.pos2data;

	hasParallax = tile.hasParallax;
	parallax = tile.parallax;

	hasScroll = tile.hasScroll;
	scrollRate = tile.scrollRate;

	tileLogic.clear();
	for (const auto& logic : tile.tileLogic) {
		tileLogic.push_back(TileLogic::create(m_context, logic->getName()));
	}

	if (tile.hasCollision) {
		set_collision(tile.hasCollision, tile.collision_border);

		collision->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision->set_on_postcontact(
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
	size = tile.size;
	std::swap(pos2data, tile.pos2data);
	chunks.swap(tile.chunks);

	hasCollision = tile.hasCollision;
	collision_border = tile.collision_border;
	std::swap(collision, tile.collision);

	hasParallax = tile.hasParallax;
	std::swap(parallax, tile.parallax);

	hasScroll = tile.hasScroll;
	std::swap(scrollRate, tile.scrollRate);

	std::swap(tileLogic, tile.tileLogic);


	if (collision) {
		collision->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}

	if (tile.collision) {
		tile.collision->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, &tile,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		tile.collision->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, &tile,
				std::placeholders::_1, std::placeholders::_2)
		);
	}


}
TileLayer& TileLayer::operator=(TileLayer&& tile) noexcept {
	m_context = tile.m_context;
	layerID = tile.layerID;
	size = tile.size;
	std::swap(pos2data, tile.pos2data);
	chunks.swap(tile.chunks);

	hasCollision = tile.hasCollision;
	collision_border = tile.collision_border;
	std::swap(collision, tile.collision);

	hasParallax = tile.hasParallax;
	std::swap(parallax, tile.parallax);

	hasScroll = tile.hasScroll;
	std::swap(scrollRate, tile.scrollRate);

	std::swap(tileLogic, tile.tileLogic);

	if (collision) {
		collision->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		collision->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, this,
				std::placeholders::_1, std::placeholders::_2)
		);
	}


	if (tile.collision) {
		tile.collision->set_on_precontact(
			std::bind(&TileLayer::handlePreContact, &tile,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);
		tile.collision->set_on_postcontact(
			std::bind(&TileLayer::handlePostContact, &tile,
				std::placeholders::_1, std::placeholders::_2)
		);
	}

	return *this;
}


TileLayer::~TileLayer() {
	if (m_context.valid() && collision) {
		//m_context.collision().get().erase_collider(collision);
		instance::phys_erase_collider(m_context, collision);
		collision = nullptr;
	}
}

void TileLayer::initFromAsset(const TileLayerRef& layerData, unsigned id) {
	clear();

	auto& tileLayer = layerData;

	layerID = id;

	size = layerData.tileSize;
	pos2data.resize((size_t)size.x * size.y, TileData{});

	for (const auto& i : tileLayer.tiles) {
		TilesetAsset* ta = Resources::get<TilesetAsset>(i.tilesetName);
		if (ta) {
			setTile(i.tilePos, i.texPos, *ta);
		}
		else {
			LOG_ERR_("unknown tileset {}", i.tilesetName);
		}
	}

	set_collision(tileLayer.has_collision, tileLayer.collision_border_bits);
	set_parallax(tileLayer.has_parallax, tileLayer.parallaxSize);
	set_scrollrate(tileLayer.has_scroll, tileLayer.scrollrate);
}

void TileLayer::set_collision(bool enabled, unsigned border) 
{
	if (enabled && (hasScroll || hasParallax)) {
		LOG_ERR_("Cannot enable collision on layer with scrolling or parallax");
		return;
	}

	if (enabled && !hasCollision) 
	{
		if (m_context.valid() && !collision) {
			collision = instance::phys_create_collider<ColliderTileMap>(m_context, Vec2i(size.x, size.y), true);
			hasCollision = true;
			collision_border = border;

			for (unsigned i = 0u; i < pos2data.size(); i++) {
				auto& tile_data = pos2data.at(i);

				if (tile_data.has_tile && tile_data.tileset_id != TILEDATA_NONE)
				{

					const TilesetAsset* tileset = chunks.at(tile_data.tileset_id).tileset;

					Tile tile = tileset->getTile(tile_data.tex_pos);

					collision->setTile(
						Vec2i{ (int)(i % size.x), (int)(i / size.x) },
						tile.shape,
						&tileset->getMaterial(tile_data.tex_pos),
						tile.matFacing
					);
				}
			}
			collision->setBorders(size, collision_border);
			collision->applyChanges();

			collision->set_on_precontact(
				std::bind(&TileLayer::handlePreContact, this,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
			);
			collision->set_on_postcontact(
				std::bind(&TileLayer::handlePostContact, this,
					std::placeholders::_1, std::placeholders::_2)
			);
		}
	}
	else if (!enabled && hasCollision)
	{
		if (m_context.valid() && collision) {
			instance::phys_erase_collider(m_context, collision);

		}
		collision = nullptr;
		hasCollision = false;
	}
}

void TileLayer::update(secs deltaTime) {
	if (collision) {
		collision->update(deltaTime);
	}
	for (auto& logic : tileLogic) {
		logic->update(deltaTime);
	}
}


void TileLayer::set_parallax(bool enabled, Vec2u parallax_size)
{
	if (enabled && hasCollision) {
		LOG_ERR_("Cannot enable parallax on layer with collision");
		return;
	}

	bool updateChunks = hasParallax != enabled;
	bool updateChunk = updateChunk || (hasParallax &&  parallax_size != parallaxSize);

	hasParallax = enabled;
	parallaxSize = parallax_size;
	if (hasParallax) {
		parallax.initOffset.x = (float)(std::min(size.x, GAME_TILE_W) * TILESIZE) / 2.f;
		parallax.initOffset.y = (float)(std::min(size.y, GAME_TILE_H) * TILESIZE) / 2.f;
		parallax.camFactor = Vec2f{ 1.f, 1.f };
		if (size.x > GAME_TILE_W) {
			parallax.camFactor.x = 1.f - ((float)(parallaxSize.x - GAME_TILE_W) / (float)size.x);
		}
		if (size.y > GAME_TILE_H) {
			parallax.camFactor.y = 1.f - ((float)(parallax_size.y - GAME_TILE_H) / (float)size.y);
		}
	}
	else {
		// reset offset on each chunk
		for (auto& vta_pair : chunks) {
			vta_pair.varray.offset = Vec2f{};
		}
	}

	for (auto& chunk : chunks) {
		chunk.varray.set_size(hasParallax ? parallaxSize : size);
	}
}
void TileLayer::set_scrollrate(bool enabled, Vec2f rate)
{
	if (enabled && hasCollision) {
		LOG_ERR_("Cannot enable scrolling on layer with collision");
		return;
	}
	hasScroll = enabled;
	scrollRate = rate;
}

bool TileLayer::handlePreContact(Vec2i pos, const Contact& contact, secs duration) {
	if (pos.x < 0 || pos.x >= size.x
		|| pos.y < 0 || pos.y >= size.y)
		return true;

	unsigned ndx = pos.y * size.x + pos.x;
	bool r = true;
	if (pos2data.at(ndx).logic_id != TILEDATA_NONE) {
		r = tileLogic.at(pos2data.at(ndx).logic_id)->on_precontact(pos, contact, duration);
	}
	return r;
}

void TileLayer::handlePostContact(Vec2i pos, const PersistantContact& contact) {
	if (pos.x < 0 || pos.x >= size.x
		|| pos.y < 0 || pos.y >= size.y)
		return;

	unsigned ndx = pos.y * size.x + pos.x;
	if (pos2data.at(ndx).logic_id != TILEDATA_NONE) {
		tileLogic.at(pos2data.at(ndx).logic_id)->on_postcontact(pos, contact);
	}
}

void TileLayer::predraw(secs deltaTime) {

	bool changed = false;

	// update logic
	for (auto& logic : tileLogic) {
		TileLogic* ptr = logic.get();
		
		while (ptr->hasNextCommand()) {

			const TileLogicCommand& cmd = ptr->nextCommand();

			unsigned ndx = cmd.position.x + size.x * cmd.position.y;
			if (cmd.type == TileLogicCommand::Type::Set) {
				setTile(cmd.position, cmd.texposition, cmd.tileset, cmd.updateLogic);
			}
			else if (pos2data.at(ndx).has_tile && cmd.type == TileLogicCommand::Type::Remove) {
				removeTile(cmd.position);
			}
			changed = true;
			
			ptr->popCommand();
		}
	}

	if (changed && hasCollision)
		collision->applyChanges();



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
	Vec2f pSize;
	if (hasParallax) {
		parallax_offset = Vec2f{};
		pSize = Vec2f{ size } *TILESIZE_F;
		if (hasParallax) {
			parallax_offset = Vec2f{
				cam_pos.x * parallax.camFactor.x,
				cam_pos.y * parallax.camFactor.y
			} - parallax.initOffset;

			for (auto& vta_pair : chunks) {
				vta_pair.varray.offset = parallax_offset;
			}
		}
	}

	// scroll update
	if (hasScroll) {
		Vec2f scroll_delta = scrollRate * deltaTime;

		for (auto& vta_pair : chunks) {
			vta_pair.varray.add_scroll(scroll_delta);
		}
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::TILELAYER_AREA))
	{
		size_t ptr = (size_t)this;
		if (!debug_draw::repeat((void*)(ptr), parallax_offset)) {
			debug_draw::set_offset(parallax_offset);
			auto& drawable1 = createDebugDrawable<ShapeRectangle, debug_draw::Type::TILELAYER_AREA>(
				(const void*)(ptr), Rectf({ 0, 0 }, pSize), Color::Transparent, Color::Red
			);
			debug_draw::set_offset();
		}
		if (!debug_draw::repeat((void*)(ptr + 1), parallax_offset + scroll_offset)) {
			debug_draw::set_offset(parallax_offset + scroll_offset);
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
	unsigned ndx = position.y * size.x + position.x;
	uint8_t tileset_ndx = pos2data.at(ndx).tileset_id;
	uint8_t logic_ndx = pos2data.at(ndx).logic_id;

	pos2data.at(ndx).tex_pos = texposition;
	pos2data.at(ndx).has_tile = true;

	if (tileset_ndx != TILEDATA_NONE) {
		chunks.at(tileset_ndx).varray.blank(position);
		pos2data.at(ndx).tileset_id = TILEDATA_NONE;
	}
	if (useLogic && logic_ndx != TILEDATA_NONE) {
		tileLogic.at(logic_ndx)->removeTile(position);
		pos2data.at(ndx).logic_id = TILEDATA_NONE;
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
							hasParallax ? parallaxSize : size, 
							Vec2u{GAME_TILE_W / 2u, GAME_TILE_H / 2u}
						)
				});

			chunks.back().varray.setTexture(tileset.getTexture());
			chunks.back().varray.setTile(position, texposition);
			chunks.back().varray.use_visible_rect = true;
			pos2data.at(ndx).tileset_id = chunks.size() - 1;
		}
		else {
			LOG_ERR_("Cannot set tile, tilelayer has reached max tileset references: {}", chunks.size());
		}
	}
	else {
		vertarr->varray.setTile(position, texposition);
		pos2data.at(ndx).tileset_id = std::distance(chunks.begin(), vertarr);
	}

	if (hasCollision) {
		Tile t = tileset.getTile(texposition);
		collision->setTile(
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
						pos2data.at(ndx).logic_id = tileLogic.size() - 1;
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
				pos2data.at(ndx).logic_id = std::distance(tileLogic.begin(), it);
			}
		}
	}
}

void TileLayer::removeTile(const Vec2u& position) {
	unsigned ndx = position.y * size.x + position.x;

	if (pos2data.at(ndx).tileset_id != TILEDATA_NONE) {
		chunks.at(pos2data.at(ndx).tileset_id).varray.blank(position);
	}
	if (pos2data.at(ndx).logic_id != TILEDATA_NONE) {
		tileLogic.at(pos2data.at(ndx).logic_id)->removeTile(position);
	}
	pos2data.at(ndx) = TileData{};

	if (hasCollision) {
		collision->removeTile(Vec2i(position));
	}
}

void TileLayer::clear() {
	pos2data.clear();
	chunks.clear();
	if (collision)
		collision->clear();
}

void TileLayer::shallow_copy(const TileLayer& layer, Rectu area, Vec2u lvlSize)
{
	set_parallax(layer.hasParallax, layer.parallaxSize);
	set_scrollrate(layer.hasScroll, layer.scrollRate);

	for (unsigned y = area.left; y < area.left + area.width; y++) {
		for (unsigned x = area.left; x < area.left + area.width; x++) {
			if (layer.size.x <= x || layer.size.y <= y)
				continue;

			Vec2u pos{ x,y };
			unsigned ndx = pos.y * layer.size.x + pos.x;
			if (!layer.pos2data.at(ndx).has_tile)
				continue;

			const TilesetAsset* tileset = nullptr;

			Vec2u tex_pos = layer.pos2data.at(ndx).tex_pos;

			if (layer.pos2data.at(ndx).tileset_id != UINT8_MAX) {
				tileset = layer.chunks.at(layer.pos2data.at(ndx).tileset_id).tileset;
			}
			if (!tileset)
				continue;

			setTile(pos, tex_pos, *tileset);
		}
	}
	set_collision(layer.hasCollision, layer.collision_border);
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
