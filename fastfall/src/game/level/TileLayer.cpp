#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/engine/config.hpp"

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


TileLayer::TileLayer(GameContext context, unsigned id, const TileLayerRef& layerData, bool initCollision)
	: m_context(context)
	, layerID(id)
{
	initFromAsset(layerData, id, initCollision);
}

TileLayer::TileLayer(const TileLayer& tile)
	: m_context(tile.m_context) 
{
	layerID = tile.layerID;
	size = tile.size;
	hasCollision = tile.hasCollision;
	collision = tile.collision;
	chunks = tile.chunks;
	pos2data = tile.pos2data;
	has_parallax = tile.has_parallax;
	parallax = tile.parallax;
	scrollRate = tile.scrollRate;

	tileLogic.clear();
	for (const auto& logic : tile.tileLogic) {
		tileLogic.push_back(TileLogic::create(m_context, logic->getName()));
	}

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

}
TileLayer& TileLayer::operator=(const TileLayer& tile) {
	m_context = tile.m_context;
	layerID = tile.layerID;
	size = tile.size;
	hasCollision = tile.hasCollision;
	collision = tile.collision;
	chunks = tile.chunks;
	pos2data = tile.pos2data;
	has_parallax = tile.has_parallax;
	parallax = tile.parallax;
	scrollRate = tile.scrollRate;

	tileLogic.clear();
	for (const auto& logic : tile.tileLogic) {
		tileLogic.push_back(TileLogic::create(m_context, logic->getName()));
	}

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

	return *this;
}
TileLayer::TileLayer(TileLayer&& tile) noexcept
	: m_context(tile.m_context)
{
	layerID = tile.layerID;
	size = tile.size;
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2data, tile.pos2data);
	chunks.swap(tile.chunks);
	has_parallax = tile.has_parallax;
	std::swap(parallax, tile.parallax);
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
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2data, tile.pos2data);
	chunks.swap(tile.chunks);
	has_parallax = tile.has_parallax;
	std::swap(parallax, tile.parallax);
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

void TileLayer::initFromAsset(const TileLayerRef& layerData, unsigned id, bool initCollision) {
	clear();

	auto& tileLayer = layerData;

	//ref = &layerData;
	hasCollision = initCollision;
	layerID = id;

	size.x = tileLayer.innerSize.x == 0 ? tileLayer.tileSize.x : tileLayer.innerSize.x;
	size.y = tileLayer.innerSize.y == 0 ? tileLayer.tileSize.y : tileLayer.innerSize.y;

	pos2data.resize(size.x * size.y, TileData{});

	has_parallax = tileLayer.has_parallax;
	scrollRate = tileLayer.scrollrate;

	// calc parallax factors
	if (has_parallax) {
		parallax.initOffset.x = (float)(std::min(size.x, GAME_TILE_W) * TILESIZE) / 2.f;
		parallax.initOffset.y = (float)(std::min(size.y, GAME_TILE_H) * TILESIZE) / 2.f;
		parallax.camFactor = Vec2f{1.f, 1.f};
		if (size.x > GAME_TILE_W) {
			parallax.camFactor.x = 1.f - ((float)(size.x - GAME_TILE_W) / (float)tileLayer.tileSize.x);
		}
		if (size.y > GAME_TILE_H) {
			parallax.camFactor.y = 1.f - ((float)(size.y - GAME_TILE_H) / (float)tileLayer.tileSize.y);
		}
	}

	if (hasCollision) {
		collision = instance::phys_create_collider<ColliderTileMap>(m_context, Vec2i(size.x, size.y), true);
	}

	for (const auto& i : tileLayer.tiles) {
		TilesetAsset* ta = Resources::get<TilesetAsset>(i.tilesetName);
		if (ta) {
			setTile(i.tilePos, i.texPos, *ta);
		}
		else {
			LOG_ERR_("unknown tileset {}", i.tilesetName);
		}
	}

	if (hasCollision) {
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

void TileLayer::update(secs deltaTime) {
	for (auto& logic : tileLogic) {
		logic->update(deltaTime);
	}
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
			if (cmd.type == TileLogicCommand::Type::Set) {
				setTile(cmd.position, cmd.texposition, cmd.tileset, cmd.updateLogic);
			}
			else if (cmd.type == TileLogicCommand::Type::Remove) {
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
	Vec2f parallax_offset;
	Vec2f pSize = Vec2f{ size } *TILESIZE_F;
	if (has_parallax) {
		parallax_offset = Vec2f{
			cam_pos.x * parallax.camFactor.x,
			cam_pos.y * parallax.camFactor.y
		} - parallax.initOffset;

		for (auto& vta_pair : chunks) {
			vta_pair.varray.offset = parallax_offset;
		}
	}

	// scroll update
	if (hasScrollX() || hasScrollY()) {
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
					.varray = ChunkVertexArray(size, Vec2u{GAME_TILE_W / 2u, GAME_TILE_H / 2u})
				});

			chunks.back().varray.setTexture(tileset.getTexture());
			chunks.back().varray.setTile(position, texposition);
			chunks.back().varray.use_visible_rect = true;
			tileset_ndx = chunks.size() - 1;
		}
		else {
			LOG_ERR_("Cannot set tile, tilelayer has reached max tileset references: {}", chunks.size());
		}
	}
	else {
		vertarr->varray.setTile(position, texposition);
		tileset_ndx = std::distance(chunks.begin(), vertarr);
	}

	if (hasCollision) {
		Tile t = tileset.getTile(texposition);
		collision->setTile(Vec2i(position), t.shape, &tileset.getMaterial(texposition), t.matFacing);
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
