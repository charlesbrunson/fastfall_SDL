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
	has_parallax = tile.has_parallax;
	parallax = tile.parallax;
	scrollRate = tile.scrollRate;

	tileLogic.clear();
	for (const auto& [type, logic] : tile.tileLogic) {
		tileLogic.insert(std::make_pair(type,
				createTileLogic(m_context, type)
			));
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
	pos2tileset = tile.pos2tileset;
	has_parallax = tile.has_parallax;
	parallax = tile.parallax;
	scrollRate = tile.scrollRate;

	tileLogic.clear();
	for (const auto& [type, logic] : tile.tileLogic) {
		tileLogic.insert(std::make_pair(type,
			createTileLogic(m_context, type)
		));
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
	std::swap(pos2tileset, tile.pos2tileset);
	tileVertices.swap(tile.tileVertices);
	has_parallax = tile.has_parallax;
	std::swap(parallax, tile.parallax);
	std::swap(scrollRate, tile.scrollRate);

	std::swap(tileLogic, tile.tileLogic);

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
	has_parallax = tile.has_parallax;
	std::swap(parallax, tile.parallax);
	std::swap(scrollRate, tile.scrollRate);

	std::swap(tileLogic, tile.tileLogic);

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

	if (hasCollision)
		collision->applyChanges();

}


void TileLayer::update(secs deltaTime) {
	for (auto& logic : tileLogic) {
		logic.second->update(deltaTime);
	}
}

void TileLayer::predraw(secs deltaTime) {

	bool changed = false;

	// update logic
	for (auto& logic : tileLogic) {
		TileLogic* ptr = logic.second.get();
		
		while (ptr->hasNextCommand()) {

			const TileLogicCommand& cmd = ptr->nextCommand();
			if (cmd.type == TileLogicCommand::Type::Set) {
				setTile(cmd.position, cmd.texposition, cmd.tileset);
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


	if (auto logic = tileset.getTileLogic(texposition); logic.has_value()) {
		auto it = tileLogic.find(logic->logicType);

		Tile t = tileset.getTile(texposition);

		if (it == tileLogic.end()) {

			auto logic_ptr = createTileLogic(m_context, logic->logicType);
			if (logic_ptr) {
				auto [n_it, inserted] = tileLogic.insert(std::make_pair(logic->logicType, std::move(logic_ptr)));
				it = n_it;


				it->second->addTile(position, t, logic->logicArg);
			}
			else {
				LOG_WARN("could not create tile logic type: {}", logic->logicType);
			}
		}
		else {
			it->second->addTile(position, t, logic->logicArg);
		}
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