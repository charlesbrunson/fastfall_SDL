#include "fastfall/game/level/Level.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/level/LevelEditor.hpp"

#include <assert.h>
#include <algorithm>

#include "imgui.h"

namespace ff {

const ActorType Level::actor_type = ActorType::create<Level>({
    .name = "Level"
});

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
                //.actor_id   = world.create<TileLayer>(entity_id, world, id_placeholder, layerRef.tilelayer),
                .actor_id   = world.create_actor<TileLayer>(layerRef.tilelayer)->id,
                .layer_id = layerRef.tilelayer.getID()
            });
		}
		else {
            layers.push_fg_front(TileLayerProxy{
                //.actor_id   = world.create<TileLayer>(entity_id, world, id_placeholder, layerRef.tilelayer),
                .actor_id   = world.create_actor<TileLayer>(layerRef.tilelayer)->id,
                .layer_id = layerRef.tilelayer.getID()
            });
		}
	}

	for (auto& layer : layers.get_tile_layers())
	{
		world.at(layer.tilelayer.actor_id).set_layer(world, layer.position);
	}

	layers.get_obj_layer().initFromAsset(
		levelData.getLayerRefs().get_obj_layer()
	);

}

void Level::resize(World& world, Vec2u n_size)
{
	for (auto& [pos, layerproxy] : layers.get_tile_layers())
	{
        auto& layer = world.at(layerproxy.actor_id);
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

        world.erase(layerproxy.actor_id);
        layerproxy.actor_id = n_layer.id;
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


void Level::ImGui_Inspect() {
    using namespace ImGui;

    ImGui::Columns(2);

    ImGui::Text("Level Name");
    ImGui::NextColumn();
    ImGui::Text("%s", levelName.c_str());
    ImGui::NextColumn();

    ImGui::Text("Asset File");
    ImGui::NextColumn();
    ImGui::Text("%s", (src_asset ? src_asset->get_path().string().c_str() : "N/A"));
    ImGui::NextColumn();

    ImGui::Text("Level Size");
    ImGui::NextColumn();
    ImGui::Text("%3d, %3d", levelSize.x, levelSize.y);
    ImGui::NextColumn();

    ImGui::Columns();

    ImGui::NewLine();
    ImGui::Text("Layers");

    for (const auto& id : layers.get_id_order()) {
        if (id != 0) {
            auto tile_layer_ptr = layers.get_tile_layer_by_id(id);
            if (!tile_layer_ptr) continue;
            auto& tile_layer = *tile_layer_ptr;
            ImGui::Text("Tile %d", tile_layer.getID());
            ImGui::Text("Tile Actor ID: %zu", tile_layer.actor_id.raw());
        }
        else {
            auto obj_layer = layers.get_obj_layer();
            ImGui::Text("Object %d", obj_layer.getID());
        }
    }
}

}