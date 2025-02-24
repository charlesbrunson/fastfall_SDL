#include "fastfall/game/level/LevelEditor.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/game/World.hpp"

#include <chrono>

namespace ff {


LevelEditor::LevelEditor(World& t_world, ID<Level> lvl)
    : world(t_world)
    , level_id(lvl)
{
}

// LAYERS

// create layer at position, selects it
bool LevelEditor::create_layer(int layer_pos)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	int bg_count = level->get_layers().get_bg_count();
	int fg_count = level->get_layers().get_fg_count();

	if (layer_pos != Level::Layers::OBJECT_LAYER_POS
		&& layer_pos >= -bg_count - 1
		&& layer_pos <= fg_count + 1)
	{
        auto ent = world.at(level_id).entity_id;
		level->get_layers().insert(
			layer_pos,
            Level::TileLayerProxy{
                //.actor_id = world.create<TileLayer>( ent, world, id_placeholder, 0, level->size() ),
                .actor_id = world.create_actor<TileLayer>(0u, level->size() )->id,
                .layer_id = 0
            }
		);
	}
	return false;
}

// select layer at positon (start and end specify the first and last layer, respectively)
bool LevelEditor::select_layer(int layer_pos)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	int bg_count = level->get_layers().get_bg_count();
	int fg_count = level->get_layers().get_fg_count();

	if (layer_pos != Level::Layers::OBJECT_LAYER_POS) {
        auto* layer = level->get_layers().get_tile_layer_at(layer_pos);
        curr_layer = layer ? std::make_optional<SelectedTileLayer>(*layer) : std::nullopt;
		obj_layer_selected = false;
	}
	else {
		obj_layer_selected = true;
	}

	if (!curr_layer) 
	{
		deselect_layer();
	}
	return curr_layer || obj_layer_selected;
}

void LevelEditor::select_obj_layer()
{
	obj_layer_selected = true;
	curr_layer = std::nullopt;
}

void LevelEditor::deselect_layer()
{
	obj_layer_selected = false;
	curr_layer = std::nullopt;
}

// move selected layer to new position
// retains selection of moved layer
bool LevelEditor::move_layer(int layer_pos)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	if (obj_layer_selected)
	{
		// ...
	}
	else if (curr_layer)
	{
		int curr_pos = curr_layer->position;
		while (curr_pos != layer_pos) {

			int n_pos = 0;
			while (curr_pos > layer_pos)
			{
				n_pos = level->get_layers().swap_prev(curr_pos);
				if (n_pos == curr_pos)
					break;

				curr_pos = n_pos;
			}
			while (curr_pos < layer_pos)
			{
				n_pos = level->get_layers().swap_next(curr_pos);
				if (n_pos == curr_pos)
					break;

				curr_pos = n_pos;
			}
		}
		return curr_pos == layer_pos;
	}
	return false;
}

// erases selected layer
// deselects layer
bool LevelEditor::erase_layer()
{
    auto* level = world.get(level_id);
	if (level && curr_layer)
	{
        world.erase(curr_layer->tile_layer_id);
		level->get_layers().erase(curr_layer->position);	
	}
	return false;
}

// LAYER PROPERTIES

bool LevelEditor::layer_set_collision(bool enabled, unsigned borderBits)
{
    auto* level = world.get(level_id);
	if (level && curr_layer) {
        auto& layer = world.at(curr_layer->tile_layer_id);
		return layer.set_collision(world, enabled, borderBits);
	}
	return false;
}
bool LevelEditor::layer_set_scroll(bool enabled, Vec2f scroll_rate)
{
    auto* level = world.get(level_id);
	if (level && curr_layer) {
        auto& layer = world.at(curr_layer->tile_layer_id);
		return layer.set_scroll(world, enabled, scroll_rate);
	}
	return false;
}
bool LevelEditor::layer_set_parallax(bool enabled, Vec2u parallax_size)
{
    auto* level = world.get(level_id);
	if (level && curr_layer) {
        auto& layer = world.at(curr_layer->tile_layer_id);
		return layer.set_parallax(world, enabled, parallax_size);
	}
	return false;
}

// TILES

// paints tile onto selected layer, using selected tileset and tile
bool LevelEditor::paint_tile(Vec2u pos)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	if (curr_layer && curr_tileset && tileset_pos) {
        auto& layer = world.at(curr_layer->tile_layer_id);
		Vec2u size = layer.getLevelSize();

		if (pos.x < size.x && pos.y < size.y) 
		{
            layer.setTile(world, pos, *tileset_pos, *curr_tileset);
			return true;
		}
	}
	return false;
}

// paints tile onto selected layer
bool LevelEditor::erase_tile(Vec2u pos)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	if (curr_layer) {
        auto& layer = world.at(curr_layer->tile_layer_id);
		Vec2u size = layer.getLevelSize();

		if (pos.x < size.x && pos.y < size.y)
		{
            layer.removeTile(world, pos);
			return true;
		}
	}
	return false;
}

// TILESET

// selects tileset for painting tiles
bool LevelEditor::select_tileset(std::string_view tileset_name)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	if (!curr_tileset || curr_tileset->get_name() != tileset_name) {
		curr_tileset = Resources::get<TilesetAsset>(tileset_name);
		deselect_tile();
	}
	return curr_tileset != nullptr;
}

bool LevelEditor::select_tileset(const TilesetAsset* tileset) 
{
    auto* level = world.get(level_id);
	if (!level) return false;

	if (!curr_tileset || curr_tileset != tileset) {
		curr_tileset = tileset;
		deselect_tile();
	}
	return curr_tileset != nullptr;
}

void LevelEditor::deselect_tileset()
{
	curr_tileset = nullptr;
	deselect_tile();
}

// selects tile from selected tileset for painting tiles
bool LevelEditor::select_tile(TileID tile)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	if (curr_tileset 
		&& tile.getX() < curr_tileset->getTileSize().x
		&& tile.getY() < curr_tileset->getTileSize().y)
	{
		tileset_pos = tile;
		return true;
	}
	return false;
}

void LevelEditor::deselect_tile()
{
	tileset_pos = std::nullopt;
}

// LEVEL PROPERTIES

// changes level's name
bool LevelEditor::set_name(std::string_view name)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	level->set_name(name);
	return true;
}

// changes level's background color
bool LevelEditor::set_bg_color(Color bg_color)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	level->set_bg_color(bg_color);
	return true;
}


bool LevelEditor::set_size(Vec2u size)
{
    auto* level = world.get(level_id);
	if (level && level->size() != size) {
		level->resize(world, size);
		return true;
	}
	return false;
}


bool LevelEditor::applyLevelAsset(const LevelAsset* asset)
{
    auto* level = world.get(level_id);
	if (!level) return false;

	auto start = std::chrono::system_clock::now();

	// step 1: level properties
	if (level->name()		!= asset->get_name())		    set_name(asset->get_name());
	if (level->size()		!= asset->getTileDimensions())	set_size(asset->getTileDimensions());
	if (level->getBGColor() != asset->getBGColor())			set_bg_color(asset->getBGColor());

	// step 2: correct layer ordering

	auto asset_ids = asset->getLayerRefs().get_id_order();

	// erase ids not present in asset_ids
	auto& lvl_layers = level->get_layers();

	auto erase_layer = [&](int i) -> bool
	{
		int id = lvl_layers.get_tile_layer_at(i)->tilelayer.getID();
		auto asset_it = std::find(asset_ids.begin(), asset_ids.end(), id);

		if (asset_it == asset_ids.end())
		{
            world.erase(lvl_layers.get_tile_layer_at(i)->tilelayer.actor_id);
			lvl_layers.erase(i);
		}
		return asset_it == asset_ids.end();
	};

	for (int i = 1; i <= lvl_layers.get_fg_count(); i++)
	{
		if (erase_layer(i)) i--;
	}
	for (int i = -1; i >= -lvl_layers.get_bg_count(); i--)
	{
		if (erase_layer(i)) i++;
	}

	// create layers not present in level
	std::set<unsigned> nLayers;
	for (auto layer_ref : asset->getLayerRefs().get_tile_layers()) 
	{
		bool not_in_lvl = std::none_of(
			lvl_layers.get_tile_layers().begin(),
			lvl_layers.get_tile_layers().end(),
			[&layer_ref](const Level::Layers::TileEntry& layer) {
				return layer_ref.tilelayer.getID() == layer.tilelayer.getID();
			}
		);
		if (not_in_lvl)
		{
			lvl_layers.push_fg_front(
                Level::TileLayerProxy{
                    .actor_id   = world.create_actor<TileLayer>(layer_ref.tilelayer )->id,
                    .layer_id = layer_ref.tilelayer.getID()
                }
            );
			nLayers.insert(layer_ref.tilelayer.getID());
		}
	}

	// reorder layers
	lvl_layers.reorder(asset_ids);

	// step 3: per layer update
	auto it = asset->getLayerRefs().get_tile_layers().begin();
	for (auto& layer : lvl_layers.get_tile_layers())
	{
		if (nLayers.contains(layer.tilelayer.getID())) {
			it++;
			continue;
		}

		assert(layer.tilelayer.getID() == it->tilelayer.getID());

		const TileLayerData& tile_ref = it->tilelayer;
		const auto& tile_ref_data = tile_ref.getTileData();

		select_layer(layer.position);

		// disable layer properties
        auto& tlayer = world.at(layer.tilelayer.actor_id);
		if (tlayer.hasCollision() && !tile_ref.hasCollision())
		{
            tlayer.set_collision(world, false);
			LOG_INFO("disable collision");
		}
		if (tlayer.hasScrolling() && !tile_ref.hasScrolling())
		{
            tlayer.set_scroll(world, false);
			LOG_INFO("disable scroll");
		}
		if (tlayer.hasParallax() && !tile_ref.hasParallax())
		{
            tlayer.set_parallax(world, false);
			LOG_INFO("disable parallax");
		}

		// enable or update layer properties
		if ((!tlayer.hasCollision() && tile_ref.hasCollision()) ||
			(tile_ref.hasCollision() && (tlayer.getCollisionBorders() != tile_ref.getCollisionBorders())))
		{
            tlayer.set_collision(world, true, tile_ref.getCollisionBorders());
			LOG_INFO("enable collision");
		}
		if ((!tlayer.hasScrolling() && tile_ref.hasScrolling()) ||
			(tile_ref.hasScrolling() && (tlayer.getScrollRate() != tile_ref.getScrollRate())))
		{
            tlayer.set_scroll(world, true, tile_ref.getScrollRate());
			LOG_INFO("enable scroll");
		}
		if ((!tlayer.hasParallax() && tile_ref.hasParallax()) ||
			(tile_ref.hasParallax() && (tlayer.getParallaxSize() != tile_ref.getParallaxSize())))
		{
            tlayer.set_parallax(world, true, tile_ref.getParallaxSize());
			LOG_INFO("enable parallax");
		}

		// update tiles
		unsigned width = tile_ref.getSize().x;
		unsigned height = tile_ref.getSize().y;

		unsigned paint_count = 0;
		unsigned erase_count = 0;
		for (auto tile_it = tile_ref_data.cbegin(); tile_it != tile_ref_data.cend(); tile_it++) 
		{
			auto& tilelayer = layer.tilelayer;
			Vec2u pos = { (unsigned)tile_it.column(), (unsigned)tile_it.row() };

			if (tile_it->has_tile)
			{
				auto tile_id = tile_it->base_id;
				auto tileset = tile_ref.getTilesetFromNdx(tile_it->tileset_ndx);

				if (   (!tlayer.hasTileAt(pos))
					|| (tlayer.getTileBaseID(pos).value() != tile_id)
					|| (tlayer.getTileTileset(pos) != tileset))
				{
					paint_count++;
					select_tileset(tileset);
					select_tile(tile_id);
					paint_tile(pos);
				}
			}
			else if (tlayer.hasTileAt(pos)) {
				erase_count++;
				erase_tile(pos);
			}
		}

		// predraw to apply changes
        tlayer.predraw(world, predraw_state_t{ .interp = 1.f, .updated = true, .update_dt = 0.0 });

		// increment to next tilelayer ref
		it++; 

		LOG_INFO("updated layer #{}: {} tile affected", layer.tilelayer.getID(), paint_count + erase_count);
	}


	// step 4: check objects



	std::chrono::duration<double> duration = std::chrono::system_clock::now() - start;
	LOG_INFO("apply duration: {}s", duration.count());


	return true;
}

}
