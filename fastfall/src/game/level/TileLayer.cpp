#include "fastfall/game/level/TileLayer.hpp"

#include "fastfall/engine/config.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/RenderTarget.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/render/ShapeRectangle.hpp"

#include "fastfall/game/level/ObjectLayer.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/World.hpp"

#include <assert.h>

namespace ff {


TileLayer::TileLayer(ID<TileLayer> t_id, unsigned id, Vec2u levelsize)
	: m_id(t_id)
    , layer_data(id, levelsize)
	, tiles_dyn(levelsize)
{
}

TileLayer::TileLayer(World& world, ID<TileLayer> t_id, const TileLayerData& layerData)
    : m_id(t_id)
{
	initFromAsset(world, layerData);
}

void TileLayer::set_layer(World& world, scene_layer lyr) {
	layer = lyr;
	for (auto chunk_id : dyn.chunks)
	{
        auto& scene_obj = world.system<SceneSystem>().config(chunk_id);
        scene_obj.layer_id = layer;
        scene_obj.resort_flag = true;
	}
}

void TileLayer::initFromAsset(World& world, const TileLayerData& layerData) {
    for (auto chunk : dyn.chunks) {
        world.erase(chunk);
    }
    if (dyn.collision.collider) {
        world.erase(*dyn.collision.collider);
    }

	layer_data = layerData;

	// init chunks
	for (auto& [tileset, _] : layer_data.getTilesets())
	{
        auto chunk_id = world.create<ChunkVertexArray>(world.entity_of(m_id), getSize(), kChunkSize);
        world.system<SceneSystem>().set_config(chunk_id, {layer, scene_type::Level});
        dyn.chunks.push_back(chunk_id);

        auto& cvr = world.at(chunk_id);
        cvr.setTexture(tileset->getTexture());
        cvr.use_visible_rect = true;
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
		auto opt_tile = tileset->getTile(tile.tile_id);

		if (!opt_tile)
			continue;

		auto chunk_id = dyn.chunks.at(tile.tileset_ndx);
        world.at(chunk_id).setTile(tile.pos, opt_tile->id);

		if (auto [logic, args] = tileset->getTileLogic(tile.tile_id); !logic.empty())
		{
			uint8_t logic_ndx = 0;
			auto logic_it = std::find_if(dyn.tile_logic.begin(), dyn.tile_logic.end(),
				[l = logic, &logic_ndx](const copyable_unique_ptr<TileLogic>& ptr) {
					logic_ndx++;
					return ptr->getName() == l;
				});

			auto& dyn_tile = tiles_dyn[tile.pos];

			if (logic_it != dyn.tile_logic.end())
			{
				dyn_tile.logic_id = logic_ndx - 1;
				logic_it->get()->addTile(tile.pos, *opt_tile, args);
			}
			else
			{
				dyn_tile.logic_id = dyn.tile_logic.size();
				dyn.tile_logic.emplace_back(TileLogic::create(world, logic));
				dyn.tile_logic.back()->addTile(tile.pos, *opt_tile, args);
			}
		}
	}

	set_collision(world, layerData.hasCollision(), layerData.getCollisionBorders());
	set_parallax(world, layerData.hasParallax(), layerData.getParallaxSize());
	set_scroll(world, layerData.hasScrolling(), layerData.getScrollRate());

	for (auto chunk_id : dyn.chunks) {
        world.at(chunk_id).predraw(1.f, true);
	}

}

void TileLayer::update(World& world, secs deltaTime) {
	if (hasCollision() && deltaTime > 0.0) {
        auto* collider = get_collider(world);
		collider->setPosition(position);
        Vec2f nV = collider->getDeltaPosition() / deltaTime;
		collider->delta_velocity = nV - collider->velocity;
		collider->velocity = nV;
	}

    for (auto chunk_id : dyn.chunks) {
        auto& cfg = world.system<SceneSystem>().config(chunk_id);
        //cfg.prev_pos = cfg.curr_pos;
        cfg.curr_pos = position;
    }


	if (hasScrolling()) {
		dyn.scroll.prev_offset = dyn.scroll.offset;
		dyn.scroll.offset += getScrollRate() * deltaTime;

		Vec2f sizef = Vec2f{ getSize() } *TILESIZE_F;

		while (dyn.scroll.offset.x < 0.f) {
			dyn.scroll.prev_offset.x += sizef.x;
			dyn.scroll.offset.x += sizef.x;
		}
		while (dyn.scroll.offset.x >= sizef.x) {
			dyn.scroll.prev_offset.x -= sizef.x;
			dyn.scroll.offset.x -= sizef.x;
		}

		while (dyn.scroll.offset.y < 0.f) {
			dyn.scroll.prev_offset.y += sizef.y;
			dyn.scroll.offset.y += sizef.y;
		}
		while (dyn.scroll.offset.y >= sizef.y) {
			dyn.scroll.prev_offset.y -= sizef.y;
			dyn.scroll.offset.y -= sizef.y;
		}
	}

	for (auto& logic : dyn.tile_logic) {
		logic->update(deltaTime);
	}
}

bool TileLayer::set_collision(World& world, bool enabled, unsigned border)
{
	if (enabled && (hasScrolling() || hasParallax())) {
		LOG_ERR_("Cannot enable collision on layer with scrolling or parallax");
		return false;
	}

	if (enabled)
	{
		if (!dyn.collision.collider) {

            dyn.collision.collider = world.create<ColliderTileMap>(world.entity_of(m_id), Vec2i{getLevelSize() }, true);
            auto* collider = get_collider(world);

			layer_data.setCollision(true, border);

			for (const auto& tile_data : layer_data.getTileData())
			{
				if (tile_data.has_tile && tile_data.tileset_ndx != TILEDATA_NONE)
				{
					const auto* tileset = layer_data.getTilesetFromNdx(tile_data.tileset_ndx);
					auto tile = tileset->getTile(tile_data.tile_id);

					if (tile) {
						collider->setTile(
							Vec2i{ tile_data.pos },
							tile->shape,
							&tileset->getMaterial(tile_data.tile_id),
							tile->matFacing
						);
					}
				}
			}

			collider->setBorders(getLevelSize(), layer_data.getCollisionBorders());
			collider->applyChanges();

            collider->set_on_precontact(
                [id = m_id, layer_id = layer_data.getID()]
                (World& w, const ContinuousContact& contact, secs duration)
                {
                    auto& tile_layer = w.at(id);
                    auto size = tile_layer.getLevelSize();
                    Vec2i pos = contact.id->quad.to_pos(size, true);

                    if (pos.x < 0 || pos.x >= size.x
                        || pos.y < 0 || pos.y >= size.y)
                        return true;

                    auto& tiles_dyn = tile_layer.tiles_dyn;

                    bool r = true;
                    if (tiles_dyn[pos].logic_id != TILEDATA_NONE) {
                        r = tile_layer.dyn.tile_logic.at(tiles_dyn[pos].logic_id)->on_precontact(w, contact, duration);
                    }
                    return r;
                }
			);
			collider->set_on_postcontact(
                [id = m_id, layer_id = layer_data.getID()]
                (World& w, const AppliedContact& contact, secs deltaTime)
                {
                    auto& tile_layer = w.at(id);
                    auto size = tile_layer.getLevelSize();
                    Vec2i pos = contact.id->quad.to_pos(size, true);

                    // based on to_pos from TileMapCollider
                    if (pos.x < 0 || pos.x >= size.x
                        || pos.y < 0 || pos.y >= size.y)
                        return;

                    auto& tiles_dyn = tile_layer.tiles_dyn;

                    //unsigned ndx = pos.y * getLevelSize().x + pos.x;
                    if (tiles_dyn[pos].logic_id != TILEDATA_NONE) {
                        tile_layer.dyn.tile_logic.at(tiles_dyn[pos].logic_id)->on_postcontact(w, contact, deltaTime);
                    }
                }
			);

		}
	}
	else if (!enabled)
	{
        if (dyn.collision.collider)
        {
            world.erase(*dyn.collision.collider);
        }
        dyn.collision.collider.reset();
        dyn.collision.is_modified = false;
		layer_data.setCollision(false);
	}
	return true;
}


bool TileLayer::set_parallax(World& world, bool enabled, Vec2u parallax_size)
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
		for (auto& chunk_id : dyn.chunks) {
			//get_chunk(world, chunk)->offset = Vec2f{};
            world.at(chunk_id).offset = Vec2f{};
		}
	}

	for (auto& chunk_id : dyn.chunks) {
        world.at(chunk_id).set_size(getSize());
	}
	return true;
}
bool TileLayer::set_scroll(World& world, bool enabled, Vec2f rate)
{
	if (enabled && hasCollision()) {
		LOG_ERR_("Cannot enable scrolling on layer with collision");
		return false;
	}

	if (layer_data.hasScrolling() && !enabled)
	{
		dyn.scroll.offset = Vec2f{};
		for (auto& chunk_id : dyn.chunks) {
            world.at(chunk_id).scroll = Vec2f{};
		}
	}

	layer_data.setScroll(enabled, rate);
	return true;
}

void TileLayer::predraw(World& world, float interp, bool updated) {
	const auto& tile_data = layer_data.getTileData();

	bool changed = false;

	// update tile logic
	if (updated) {
		for (auto& logic : dyn.tile_logic) {
			TileLogic* ptr = logic.get();

			while (ptr->hasNextCommand()) {

				const TileLogicCommand& cmd = ptr->nextCommand();

				if (cmd.type == TileLogicCommand::Type::Set) {
					setTile(world, cmd.position, cmd.texposition, cmd.tileset, cmd.updateLogic);
				}
				else if (tile_data[cmd.position].has_tile && cmd.type == TileLogicCommand::Type::Remove) {
					removeTile(world, cmd.position);
				}
				changed = true;

				ptr->popCommand();
			}
		}
	}
    if (hasCollision() && dyn.collision.is_modified) {
        get_collider(world)->applyChanges();
        dyn.collision.is_modified = false;
    }

    // calc visible area
	Rectf visible;
	Vec2f cam_pos = world.system<CameraSystem>().getPosition(interp); //instance::cam_get_interpolated_pos(m_context, interp);
	float cam_zoom = world.system<CameraSystem>().zoomFactor; //instance::cam_get_zoom(m_context);
	visible.width = GAME_W_F * cam_zoom;
	visible.height = GAME_H_F * cam_zoom;
	visible.left = cam_pos.x - (visible.width / 2.f);
	visible.top = cam_pos.y - (visible.height / 2.f);

    // update parallax offset
	if (hasParallax()) {
		dyn.parallax.offset = Vec2f{ 
			cam_pos.x * dyn.parallax.cam_factor.x,
			cam_pos.y * dyn.parallax.cam_factor.y
		} - dyn.parallax.init_offset;
	}

    for (auto& chunk_id : dyn.chunks) {
        auto& chunk = world.at(chunk_id);
        chunk.visibility = visible;

        // parallax update
        if (hasParallax()) {
            chunk.offset = dyn.parallax.offset;
        }

        // scroll update
        if (hasScrolling()) {
            chunk.scroll = math::lerp(dyn.scroll.prev_offset, dyn.scroll.offset, interp);
        }

        // chunk predraw
        chunk.predraw(interp, updated);
    }

	if (debug_draw::hasTypeEnabled(debug_draw::Type::TILELAYER_AREA) && updated)
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
}

void TileLayer::setTile(World& world, const Vec2u& position, TileID tile_id, const TilesetAsset& tileset, bool useLogic)
{

	auto& tile_data = layer_data.getTileData();
	auto prev_tileset_ndx = tile_data[position].tileset_ndx;

	auto changes = layer_data.setTile(position, tile_id, tileset);

	for (int i = 0; i < changes.count; i++) {
		auto& change = changes.arr[i];
		auto next_tileset_ndx = tile_data[change.position].tileset_ndx;

		updateTile(
            world,
			change.position, 
			change.position == position ? prev_tileset_ndx : tile_data[change.position].tileset_ndx, 
			change.tileset, 
			useLogic);
	}
}



void TileLayer::removeTile(World& world, const Vec2u& position) {
	const auto& tile_data = layer_data.getTileData();

	if (tile_data[position].tileset_ndx != TILEDATA_NONE) {
		uint8_t t_ndx = tile_data[position].tileset_ndx;
        // TODO buffer changes?
        world.at(dyn.chunks.at(t_ndx)).blank(position);
	}
	if (tiles_dyn[position].logic_id != TILEDATA_NONE) {
		dyn.tile_logic.at(tiles_dyn[position].logic_id)->removeTile(position);
	}
	if (hasCollision()) {
        // TODO buffer changes?
        get_collider(world)->removeTile(Vec2i(position));
        dyn.collision.is_modified = true;
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
            world,
			change.position,
			tile_data[change.position].tileset_ndx,
			change.tileset,
			true);
	}

}

void TileLayer::updateTile(World& world, const Vec2u& at, uint8_t prev_tileset_ndx, const TilesetAsset* next_tileset, bool useLogic)
{
	uint8_t tileset_ndx = prev_tileset_ndx;
	if (tileset_ndx != TILEDATA_NONE) {
        // TODO buffer changes?
        world.at(dyn.chunks.at(tileset_ndx)).blank(at);
	}

	uint8_t logic_ndx = tiles_dyn[at].logic_id;
	if (useLogic && logic_ndx != TILEDATA_NONE) {
		dyn.tile_logic.at(logic_ndx)->removeTile(at);
		tiles_dyn[at].logic_id = TILEDATA_NONE;
	}

	auto& tile = layer_data.getTileData()[at];

	if (!next_tileset || !next_tileset->getTile(tile.tile_id)) 
	{
		if (hasCollision())
		{
            // TODO buffer changes?
            get_collider(world)->removeTile(Vec2i(at));
            dyn.collision.is_modified = true;
		}
		return;
	}

	std::optional<Tile> next_tile = next_tileset->getTile(tile.tile_id);

	if (tile.tileset_ndx == dyn.chunks.size())
	{
        // TODO buffer changes?
        auto chunk_id = world.create<ChunkVertexArray>(world.entity_of(m_id), getSize(), kChunkSize);
        world.system<SceneSystem>().set_config(chunk_id, { layer, scene_type::Level });

        auto& cvr = world.at(chunk_id);
        cvr.setTexture(next_tileset->getTexture());
        cvr.setTile(at, tile.tile_id);
        cvr.use_visible_rect = false;

        dyn.chunks.push_back(chunk_id);
	}
	else {
        // TODO buffer changes?
        world.at(dyn.chunks.at(tile.tileset_ndx)).setTile(at, tile.tile_id);
	}

	if (hasCollision() && next_tile) {

        // TODO buffer changes?
        get_collider(world)->setTile(
			Vec2i(at),
			next_tile->shape,
			&next_tileset->getMaterial(tile.tile_id),
			next_tile->matFacing
		);
        dyn.collision.is_modified = true;
	}

	if (useLogic) {
		if (auto [logic, args] = next_tileset->getTileLogic(tile.tile_id); !logic.empty()) {
			std::string_view logic_var = logic;

			unsigned dist = 0;
			auto it = std::find_if(dyn.tile_logic.begin(), dyn.tile_logic.end(),
				[logic_var, &dist](const copyable_unique_ptr<TileLogic>& log) {
					dist++;
					return logic_var == log->getName();
				});
			dist--;

			if (it == dyn.tile_logic.end()) {
				if (dyn.tile_logic.size() < UINT8_MAX - 1) {

					auto logic_ptr = TileLogic::create(world, logic);
					if (logic_ptr) {
						dyn.tile_logic.push_back(std::move(logic_ptr));
						dyn.tile_logic.back()->addTile(at, *next_tile, args.data());
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
				it->get()->addTile(at, *next_tile, args.data());
				tiles_dyn[at].logic_id = dist;
			}
		}
	}
}

void TileLayer::shallow_copy(World& world, const TileLayer& src, Rectu src_area, Vec2u dst)
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
		setTile(world, tile.pos - dst, tex_pos, *tileset);
	}
}


bool TileLayer::hasTileAt(Vec2u tile_pos) const
{
	if (tile_pos.x < getLevelSize().x && tile_pos.y < getLevelSize().y)
	{
		return layer_data.getTileData()[tile_pos].has_tile;
	}
	return false;
}

std::optional<TileID> TileLayer::getTileID(Vec2u tile_pos) const
{
	if (hasTileAt(tile_pos))
	{
		return layer_data.getTileData()[tile_pos].tile_id;
	}
	return std::nullopt;
}

std::optional<TileID> TileLayer::getTileBaseID(Vec2u tile_pos) const
{
	if (hasTileAt(tile_pos))
	{
		return layer_data.getTileData()[tile_pos].base_id;
	}
	return std::nullopt;
}

const TilesetAsset* TileLayer::getTileTileset(Vec2u tile_pos) const
{
	if (hasTileAt(tile_pos))
	{
		return layer_data.getTilesetFromNdx(layer_data.getTileData()[tile_pos].tileset_ndx);
	}
	return nullptr;
}

bool TileLayer::isTileAuto(Vec2u tile_pos) const
{
	if (hasTileAt(tile_pos))
	{
		return layer_data.getTileData()[tile_pos].is_autotile;
	}
	return false;
}

TileShape TileLayer::getTileShape(Vec2u tile_pos) const
{
	if (hasTileAt(tile_pos))
	{
		return layer_data.getTileShapes()[tile_pos];
	}
	return TileShape{};
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

ColliderTileMap* TileLayer::get_collider(World& world) {
    return world.get(*dyn.collision.collider);
}

}
