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
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	collision = tile.collision;
	tileVertices = tile.tileVertices;
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
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2data, tile.pos2data);
	tileVertices.swap(tile.tileVertices);
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
	ref = tile.ref;
	size = tile.size;
	hasCollision = tile.hasCollision;
	std::swap(collision, tile.collision);
	std::swap(pos2data, tile.pos2data);
	tileVertices.swap(tile.tileVertices);
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

void TileLayer::initFromAsset(const LayerRef& layerData, bool initCollision) {
	clear();
	assert(layerData.type == LayerType::TILELAYER);

	ref = &layerData;
	hasCollision = initCollision;
	layerID = layerData.id;

	size.x = layerData.tileLayer->innerSize.x == 0 ? layerData.tileLayer->tileSize.x : layerData.tileLayer->innerSize.x;
	size.y = layerData.tileLayer->innerSize.y == 0 ? layerData.tileLayer->tileSize.y : layerData.tileLayer->innerSize.y;

	pos2data.resize(size.x * size.y, TileData{});

	has_parallax = layerData.tileLayer->has_parallax;
	scrollRate = layerData.tileLayer->scrollrate;

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
		}
		else {
			LOG_ERR_("unknown tileset {}", *i.second.tilesetName);
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
	unsigned ndx = pos.y * size.x + pos.x;
	bool r = true;
	if (pos2data.at(ndx).logic_id != TILEDATA_NONE) {
		r = tileLogic.at(pos2data.at(ndx).logic_id)->on_precontact(pos, contact, duration);
	}
	return r;
}

void TileLayer::handlePostContact(Vec2i pos, const PersistantContact& contact) {
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
			for (auto& vta_pair : tileVertices)	vta_pair.varray.rotate_backwardX();
		}
		while (scroll_offset.x >= TILESIZE_F) {
			scroll_offset.x -= TILESIZE_F;
			for (auto& vta_pair : tileVertices)	vta_pair.varray.rotate_forwardX();
		}
		while (scroll_offset.y < 0.f) { 
			scroll_offset.y += TILESIZE_F;
			for (auto& vta_pair : tileVertices)	vta_pair.varray.rotate_backwardY();
		}
		while (scroll_offset.y >= TILESIZE_F) {
			scroll_offset.y -= TILESIZE_F;
			for (auto& vta_pair : tileVertices)	vta_pair.varray.rotate_forwardY();
		}

	}

	if (has_parallax || hasScrollX() || hasScrollY()) {
		for (auto& vta_pair : tileVertices) {
			vta_pair.varray.offset = parallax_offset + scroll_offset;
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

void TileLayer::setTile(const Vec2u& position, const Vec2u& texposition, const TilesetAsset& tileset, bool useLogic) {

	// blank existing tile at that position
	unsigned ndx = position.y * size.x + position.x;
	uint8_t tileset_ndx = pos2data.at(ndx).tileset_id;
	uint8_t logic_ndx = pos2data.at(ndx).logic_id;
	if (tileset_ndx != TILEDATA_NONE) {
		tileVertices.at(tileset_ndx).varray.blank(position);
		pos2data.at(ndx).tileset_id = TILEDATA_NONE;
	}
	if (useLogic && logic_ndx != TILEDATA_NONE) {
		tileLogic.at(logic_ndx)->removeTile(position);
		pos2data.at(ndx).logic_id = TILEDATA_NONE;
	}
	//pos2data.at(ndx) = TileData{};

	// find tilearray or create it
	auto vertarr = std::find_if(tileVertices.begin(), tileVertices.end(), [&tileset](const TVArrayT& tva) {
		return tva.tileset == &tileset;
		});
	if (vertarr == tileVertices.end()) {

		if (tileVertices.size() < UINT8_MAX) {

			tileVertices.push_back(TVArrayT{
					.tileset = &tileset,
					.varray = TileVertexArray(size)
				});

			tileVertices.back().varray.setTexture(tileset.getTexture());
			tileVertices.back().varray.setTile(position, texposition);
			tileset_ndx = tileVertices.size() - 1;
		}
		else {
			LOG_ERR_("Cannot set tile, tilelayer has reached max tileset references: {}", tileVertices.size());
		}
	}
	else {
		vertarr->varray.setTile(position, texposition);
		tileset_ndx = std::distance(tileVertices.begin(), vertarr);
	}

	// set new tiledata
	//pos2data.at(ndx) = TileData{};

	if (hasCollision) {
		Tile t = tileset.getTile(texposition);
		collision->setTile(Vec2i(position), t.shape, &tileset.getMaterial(texposition), t.matFacing);
	}

	if (useLogic) {

		if (auto logic = tileset.getTileLogic(texposition); logic.has_value()) {
			auto it = std::find_if(tileLogic.begin(), tileLogic.end(), [&logic](const std::unique_ptr<TileLogic>& log) {
				return logic->logicType == log->getName();
				});

			Tile t = tileset.getTile(texposition);

			if (it == tileLogic.end()) {

				auto logic_ptr = TileLogic::create(m_context, logic->logicType);
				if (logic_ptr) {
					tileLogic.push_back(std::move(logic_ptr));
					tileLogic.back()->addTile(position, t, logic->logicArg);
					pos2data.at(ndx).logic_id = tileLogic.size() - 1;
				}
				else {
					LOG_WARN("could not create tile logic type: {}", logic->logicType);
				}
			}
			else {
				it->get()->addTile(position, t, logic->logicArg);
				pos2data.at(ndx).logic_id = std::distance(tileLogic.begin(), it);
			}
		}

	}

}
void TileLayer::removeTile(const Vec2u& position) {
	//auto iter1 = pos2tileset.find(position);

	unsigned ndx = position.y * size.x + position.x;

	if (pos2data.at(ndx).tileset_id != TILEDATA_NONE) {
		tileVertices.at(pos2data.at(ndx).tileset_id).varray.erase(position);
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
	ref = nullptr;
	pos2data.clear();
	tileVertices.clear();
	if (collision)
		collision->clear();
}

void TileLayer::draw(RenderTarget& target, RenderState states) const {
	states.transform = Transform::combine(states.transform, Transform(offset));

	if (!hidden) {
		for (auto& layer : tileVertices) {
			states.texture = layer.varray.getTexture();

			target.draw(layer.varray, states);
		}
	}
}

}