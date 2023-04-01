#include "fastfall/game/level/Level.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/level/LevelEditor.hpp"

#include <assert.h>
#include <algorithm>

#include "imgui.h"

namespace ff {

const ActorType Level::actor_type {
    .name = "Level"
};

// LEVEL
Level::Level(
    ActorInit init,
    std::optional<std::string>  opt_name,
    std::optional<Vec2u>        opt_size,
    std::optional<Color>        opt_bgColor
)
    : Actor{init.type_or(&actor_type) }
{
    if (opt_name)    levelName = std::move(*opt_name);
    if (opt_size)    levelSize = *opt_size;
    if (opt_bgColor) bgColor = *opt_bgColor;
}

Level::Level(
    ActorInit init,
    const LevelAsset& levelData
)
    : Actor{init.type_or(&actor_type) }
{
    initFromAsset(init.world, levelData);
}

void Level::initFromAsset(World& world, const LevelAsset& levelData)
{
    unsubscribe_all_assets();
    subscribe_asset(&levelData);
    src_asset = &levelData;

    // remove existing components and layers
    world.erase_all_components(entity_id);
    layers.clear_all();

	levelName = levelData.get_name();
	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();

	for (auto& layerRef : levelData.getLayerRefs().get_tile_layers())
	{
		if (layerRef.position < 0) {
            layers.push_bg_front(TileLayerProxy{
                //.cmp_id   = world.create<TileLayer>(entity_id, world, id_placeholder, layerRef.tilelayer),
                .cmp_id   = world.create_actor<TileLayer>(layerRef.tilelayer)->id,
                .layer_id = layerRef.tilelayer.getID()
            });
		}
		else {
            layers.push_fg_front(TileLayerProxy{
                //.cmp_id   = world.create<TileLayer>(entity_id, world, id_placeholder, layerRef.tilelayer),
                .cmp_id   = world.create_actor<TileLayer>(layerRef.tilelayer)->id,
                .layer_id = layerRef.tilelayer.getID()
            });
		}
	}

	for (auto& layer : layers.get_tile_layers())
	{
		world.at(layer.tilelayer.cmp_id).set_layer(world, layer.position);
	}

	layers.get_obj_layer().initFromAsset(
		levelData.getLayerRefs().get_obj_layer()
	);

}

void Level::resize(World& world, Vec2u n_size)
{
	for (auto& [pos, layerproxy] : layers.get_tile_layers())
	{
        auto& layer = world.at(layerproxy.cmp_id);
		Vec2u layer_size{
            (std::min)(n_size.x, layer.getLevelSize().x),
            (std::min)(n_size.y, layer.getLevelSize().y)
		};

		Vec2u parallax_size{
            (std::min)(n_size.x, layer.getParallaxSize().x),
            (std::min)(n_size.y, layer.getParallaxSize().y)
		};

        //auto ent = world.entity_of(m_id);
        //auto n_layer = world.create<TileLayer>(entity_id, world, id_placeholder, layer.getID(), n_size);
        auto n_layer = *world.create_actor<TileLayer>(layer.getID(), n_size);
        n_layer->set_layer(world, layer.get_layer());
        n_layer->set_collision(world, layer.hasCollision(), layer.getCollisionBorders());
        n_layer->set_scroll(world, layer.hasScrolling(), layer.getScrollRate());
        n_layer->set_parallax(world, layer.hasParallax(), parallax_size);
        n_layer->shallow_copy(world, layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, {0, 0});

        world.erase(layerproxy.cmp_id);
        layerproxy.cmp_id = n_layer.id;
	}
	levelSize = n_size;
}

bool Level::try_reload_level(World& w) {
    if (!src_asset || !allow_asset_reload || !asset_changed)
        return false;

    LevelEditor editor{ w, id_cast<Level>(actor_id) };
    bool r = editor.applyLevelAsset(src_asset);
    if (r)
        asset_changed = false;
    return r;
}

}
