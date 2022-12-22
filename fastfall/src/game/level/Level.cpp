#include "fastfall/game/level/Level.hpp"

#include "fastfall/game/World.hpp"

#include <assert.h>

#include "imgui.h"

namespace ff {

// LEVEL


Level::Level(
        World& world,
        ID<Level> t_id,
        std::optional<std::string>  opt_name,
        std::optional<Vec2u>        opt_size,
        std::optional<Color>        opt_bgColor)
    : m_id(t_id)
{
    auto ent = world.entity_of(t_id);
    auto cmps = world.components_of(ent);
    for (auto& c : cmps) {
        if (std::holds_alternative<ID<Level>>(c) && c != ComponentID{ t_id })
        {
            LOG_ERR_("Entity {} has multiple instances of Level component!", ent.value.sparse_index);
        }
        if (std::holds_alternative<ID<GameObject>>(c) && c != ComponentID{ t_id })
        {
            LOG_ERR_("Entity {} has both a Level and GameObject component!", ent.value.sparse_index);
        }
    }
    if (opt_name)    levelName = std::move(*opt_name);
    if (opt_size)    levelSize = *opt_size;
    if (opt_bgColor) bgColor = *opt_bgColor;
}

Level::Level(World& world, ID<Level> t_id, const LevelAsset& levelData)
    : m_id(t_id)
{
    auto ent = world.entity_of(t_id);
    auto cmps = world.components_of(ent);
    for (auto& c : cmps) {
        if (std::holds_alternative<ID<Level>>(c) && c != ComponentID{ t_id })
        {
            LOG_ERR_("Entity {} has multiple instances of Level component!", ent.value.sparse_index);
        }
        if (std::holds_alternative<ID<GameObject>>(c) && c != ComponentID{ t_id })
        {
            LOG_ERR_("Entity {} has both a Level and GameObject component!", ent.value.sparse_index);
        }
    }

    initFromAsset(world, levelData);
}

/*
void Level::update(World& world, secs deltaTime) {
    for (auto& [pos, layer] : layers.get_tile_layers()) {
        world.at(layer.cmp_id).update(world, deltaTime);
    }
}
*/

/*
void Level::predraw(World& world, float interp, bool updated) {
    for (auto& [pos, layer] : layers.get_tile_layers()) {
        world.at(layer.cmp_id).predraw(world, interp, updated);
    }
}
*/

void Level::initFromAsset(World& world, const LevelAsset& levelData)
{
    // remove existing components and layers
    auto entity = world.entity_of(getID());
    auto components = world.components_of(entity);
    for (auto c : components) {
        if (c != ComponentID{ getID() }) {
            world.erase(c);
        }
    }
    layers.clear_all();

	levelName = levelData.getAssetName();
	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();

	for (auto& layerRef : levelData.getLayerRefs().get_tile_layers())
	{
        auto ent = world.entity_of(m_id);
		if (layerRef.position < 0) {
            layers.push_bg_front(TileLayerProxy{
                .cmp_id   = world.create<TileLayer>(ent, world, id_placeholder, layerRef.tilelayer),
                .layer_id = layerRef.tilelayer.getID()
            });
		}
		else {
            layers.push_fg_front(TileLayerProxy{
                .cmp_id   = world.create<TileLayer>(ent, world, id_placeholder, layerRef.tilelayer),
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
			std::min(n_size.x, layer.getLevelSize().x),
			std::min(n_size.y, layer.getLevelSize().y)
		};

		Vec2u parallax_size{
			std::min(n_size.x, layer.getParallaxSize().x),
			std::min(n_size.y, layer.getParallaxSize().y)
		};

        auto ent = world.entity_of(m_id);
        auto n_id = world.create<TileLayer>(ent, world, id_placeholder, layer.getID(), n_size);
        TileLayer& n_layer = world.at(n_id);

        n_layer.set_layer(world, layer.get_layer());
        n_layer.set_collision(world, layer.hasCollision(), layer.getCollisionBorders());
        n_layer.set_scroll(world, layer.hasScrolling(), layer.getScrollRate());
        n_layer.set_parallax(world, layer.hasParallax(), parallax_size);
        n_layer.shallow_copy(world, layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, {0, 0});

        world.erase(layerproxy.cmp_id);
        layerproxy.cmp_id = n_id;
	}
	levelSize = n_size;
}

}
