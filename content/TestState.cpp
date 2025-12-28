
#include "TestState.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/render/drawable/ShapeRectangle.hpp"
#include "fastfall/render/drawable/ShapeCircle.hpp"
#include "fastfall/render/drawable/ShapeLine.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/engine/input/InputConfig.hpp"

#include "fastfall/engine/input/Mouse.hpp"

#include "tilelogic/AnimLogic.hpp"
#include "fastfall/engine/Engine.hpp"
#include "object/Player.hpp"

#include <fstream>

#include "SDL3/SDL.h"

TestState::TestState()
    : insrc_realtime(input_sets::gameplay, 1.0/60.0, RecordInputs::Yes)
{
	stateID = EngineStateID::TEST_STATE;
	clearColor = Color{ 0x141013FF };

    world = std::make_unique<World>();
    world->input().set_source(&insrc_realtime);
    on_realtime = true;

    ID<Level> lvl_id;
	if (auto* lvl_asset = Resources::get<LevelAsset>("map_test.tmx"))
	{
        lvl_id = world->create_actor<Level>(*lvl_asset)->id;
        world->system<LevelSystem>().set_active(lvl_id);
	}
    Level* lvl = world->system<LevelSystem>().get_active(*world);
    lvl->get_obj_layer().createActorsFromObjects(*world);

	edit = std::make_unique<LevelEditor>( *world, lvl_id );
	edit->select_layer(-1);
	edit->select_tileset("tech_fg.tsx");
	edit->select_tile(edit->get_tileset()->getAutoTileForShape("slope"_ts).value_or(TileID{0u, 0u}));

	auto font = Resources::get<FontAsset>("LionelMicroNbp-gA25.ttf");
	tile_text.setText(*font, 8, {});

    save_world = std::make_unique<World>(*world);

    world->name      = "current";
    save_world->name = "saved";
}

void TestState::update(secs deltaTime) {

    if (deltaTime > 0.0) {
        world->update(deltaTime);
        if (auto src = world->input().get_source()) {
            src->next();
        }
    }

	if (edit)
	{
        currKeys = SDL_GetKeyboardState(&key_count);
		const TileLayer& tilelayer = world->at(edit->get_tile_layer()->tile_layer_id);

		mpos = Mouse::world_pos();
		tpos = tilelayer.getTileFromWorldPos(mpos).value_or(Vec2i{});

		auto onKeyPressed = [this](SDL_Scancode c, auto&& callable) {
			if (currKeys && prevKeys && currKeys[c] && !prevKeys[c]) {
				callable();
			}
		};

		auto tileOnKeyPressed = [&](SDL_Scancode c, Vec2i dir) {
			onKeyPressed(c, [this, dir]() {
				auto tileset = edit->get_tileset();
				auto tile = edit->get_tile();
				if (tileset && tile) {
					TileID tile_pos = tile.value();
					tile_pos.setX((tile_pos.getX() + dir.x) % tileset->getTileSize().x);
					tile_pos.setY((tile_pos.getY() + dir.y) % tileset->getTileSize().y);
					edit->select_tile(tile_pos);
				}
				else {
					edit->select_tile(TileID{ 0u, 0u });
				}
			});
		};
		auto layerOnKeyPressed = [&](SDL_Scancode c, int i) {
			onKeyPressed(c, [this, i]() {
				int curr_layer = edit->get_tile_layer()->position;

				int n_layer = curr_layer + i;
				if (n_layer == 0) {
					n_layer = n_layer + (i > 0 ? 1 : -1);
				}

				if (!edit->select_layer(n_layer)) {
					edit->select_layer(curr_layer);
				}
				layer = edit->get_tile_layer()->position;
			});
		};

		tileOnKeyPressed(SDL_SCANCODE_LEFT,  Vec2i{ -1,  0 });
		tileOnKeyPressed(SDL_SCANCODE_RIGHT, Vec2i{  1,  0 });
		tileOnKeyPressed(SDL_SCANCODE_UP,    Vec2i{  0, -1 });
		tileOnKeyPressed(SDL_SCANCODE_DOWN,  Vec2i{  0,  1 });

		layerOnKeyPressed(SDL_SCANCODE_KP_MINUS, -1);
		layerOnKeyPressed(SDL_SCANCODE_KP_PLUS, 1);

        onKeyPressed(SDL_SCANCODE_F1, [&]() { to_save = true; });
        onKeyPressed(SDL_SCANCODE_F2, [&]() { to_load = true; on_realtime = true; });
        onKeyPressed(SDL_SCANCODE_F3, [&]() { to_load = true; on_realtime = false; });
		onKeyPressed(SDL_SCANCODE_F4, [&]() {
			createState<TestState>();
			setEngineAction(EngineStateAction::SWAP_NEXT);
		});

		onKeyPressed(SDL_SCANCODE_C, [&]() {
				auto tilelayer = edit->get_tile_layer();

				Vec2u tile_pos = Vec2u{ tpos };

				if (tilelayer
					&& world->at(tilelayer->tile_layer_id).hasTileAt(tile_pos))
				{
					const TilesetAsset* tileset = world->at(tilelayer->tile_layer_id).getTileTileset(tile_pos);
					TileID id = world->at(tilelayer->tile_layer_id).getTileBaseID(tile_pos).value_or(TileID{});

					if (tileset && id.valid())
					{
						edit->select_tileset(tileset);
						edit->select_tile(id);
					}
				}
			});

		onKeyPressed(SDL_SCANCODE_F, [&]() {
				if (edit->get_tile_layer()
					&& edit->get_tileset()
					&& edit->get_tile()
					&& edit->get_tileset()->getTile(*edit->get_tile())
					&& edit->get_tileset()->getTile(*edit->get_tile())->auto_substitute)
				{	
					auto shape = edit->get_tileset()->getTile(*edit->get_tile())->shape;
					shape = TileShape{ shape.type, !shape.flip_h, shape.flip_v };

					if (auto flip_id = edit->get_tileset()->getAutoTileForShape(shape))
					{
						edit->select_tile(flip_id.value());
					}
				}
			});

		onKeyPressed(SDL_SCANCODE_V, [&]() {
			if (edit->get_tile_layer()
				&& edit->get_tileset()
				&& edit->get_tile()
				&& edit->get_tileset()->getTile(*edit->get_tile())
				&& edit->get_tileset()->getTile(*edit->get_tile())->auto_substitute)
			{
				auto shape = edit->get_tileset()->getTile(*edit->get_tile())->shape;
				shape = TileShape{ shape.type, shape.flip_h, !shape.flip_v };

				if (auto flip_id = edit->get_tileset()->getAutoTileForShape(shape))
				{
					edit->select_tile(flip_id.value());
				}
			}
			});

		if (Mouse::in_view() && (Mouse::m1.is_held() || Mouse::m2.is_held()))
		{
            Level* lvl = world->system<LevelSystem>().get_active(*world);
			if (Rectf{ Vec2f{}, Vec2f{lvl->size()} * TILESIZE_F }.contains(mpos)
				&& (!painting || (last_paint != tpos)))
			{
				Mouse::m1.is_held()
					? edit->paint_tile(Vec2u{ tpos })
					: edit->erase_tile(Vec2u{ tpos });
			}

			last_paint = tpos;
			painting = true;
		}
		else {
			painting = false;
		}

		auto tileset = edit->get_tileset();
		auto tile_id = edit->get_tile();


		if (tileset && tile_id) {
            auto tile = tileset->getTile(*tile_id);

            std::string_view tileset_name = tileset->get_name();
            std::string_view layer_name = world->at(edit->get_tile_layer()->tile_layer_id).getName();
            unsigned layer_id = edit->get_tile_layer()->layer_id;
            int layer_pos = layer;

            Vec2u tile_origin = tile_id->to_vec();
            std::string_view tile_type = (tile ? (tile->auto_substitute ? "auto" : "") : "null");

            std::string str =
                    fmt::format("tileset\t{}\n", tileset_name)
                    + fmt::format("layer\t\t{}#{} ({})\n", layer_name, layer_id, layer_pos)
                    + fmt::format("tile\t\t\t{:2d}\t{}\n", tile_origin, tile_type)
                    + fmt::format("pos\t\t{:3d}\n", tpos)
                    + fmt::format("pixel\t\t{:4d}\n", Vec2i{mpos});


            tile_text.setText({}, {}, str);

        }
	}

	if (currKeys) {
		if (!prevKeys)
			prevKeys = std::make_unique<bool[]>(key_count);
		std::memcpy(&prevKeys[0], currKeys, key_count);
	}
}

void TestState::predraw(predraw_state_t predraw_state, const WindowState* win_state) {

    if (to_save) {
        if (save_world) {
            *save_world = *world;
        }
        else {
            save_world = std::make_unique<World>(*world);
        }
        to_save = false;
        LOG_INFO("saved state");
    }
    else if (to_load) {
        if (save_world) {

            *world = *save_world;

            if (on_realtime) {
                auto record = *insrc_realtime.get_record();
                record.frame_data.resize(world->tick_count(), {});

                insrc_realtime.set_record(record);
                world->input().set_source(&insrc_realtime);
            }
            else {
                insrc_record = InputSourceRecord{ *insrc_realtime.get_record(), world->tick_count() };
                world->input().set_source(&*insrc_record);
            }

            debug::reset();
        }
        to_load = false;
        LOG_INFO("loaded state");
    }

    world->predraw(predraw_state);
	viewPos = world->system<CameraSystem>().getPosition(predraw_state.interp);
	viewZoom = world->system<CameraSystem>().zoomFactor;

	tile_text.predraw();

    show_tile = Mouse::in_view();

	if (edit && win_state)
	{
		auto tileset = edit->get_tileset();
		auto tile = edit->get_tile();

		if (tileset && tile) 
		{

			const TileLayer& tilelayer = world->at(edit->get_tile_layer()->tile_layer_id);
			ghost_pos = tilelayer.getWorldPosFromTilePos(tpos);

			tile_ghost.setPosition(ghost_pos.real);
			tile_ghost.setSize({ TILESIZE_F, TILESIZE_F });
			tile_ghost.setColor(Color::White().alpha(80));
			tile_ghost.setTexture(&tileset->getTexture());
			tile_ghost.setTextureRect(Rectf{
					Vec2f{ tile->to_vec() } * TILESIZE_F,
					Vec2f{ 1, 1 } * TILESIZE_F
				});


			float win_scale = win_state->scale;
			float scale = viewZoom / win_scale;
			tile_text.setScale( Vec2f{ 1.f, 1.f } * scale * (win_scale > 2 ? 2.f : 1.f) );
			tile_text.setColor(Color::White);

			float posx, posy;
			SDL_GetMouseState(&posx, &posy);

			Vec2f mouse_pos { Mouse::window_pos() };
			Vec2f win_size  { win_state->window_size };
			Vec2f text_off  { 0.f, -tile_text.getScaledBounds().height };

			Vec2f mouse_from_cam = (mouse_pos - (win_size / 2.f));
			mouse_from_cam.x = floorf(mouse_from_cam.x);
			mouse_from_cam.y = floorf(mouse_from_cam.y);
			mouse_from_cam /= win_scale;

			Vec2f text_pos = viewPos + text_off + mouse_from_cam * viewZoom;
			
			tile_text.setPosition(text_pos);

		}
	}
	else {
		tile_ghost.setColor(Color::Transparent);
		tile_text.setColor(Color::Transparent);
	}
}

bool TestState::pushEvent(const SDL_Event& event) {

    if (on_realtime) {
        return insrc_realtime.push_event(event);
    }
    return false;
}

void TestState::draw(RenderTarget& target, RenderState state) const
{
    target.draw(*world,         state);

	if (show_tile && edit && edit->get_tile_layer()) {
		target.draw(tile_ghost, state);
		target.draw(tile_text, state);

		Vec2f offset = -1.f * Vec2f{ world->at(edit->get_tile_layer()->tile_layer_id).getSize() } * TILESIZE_F;

		if (world->at(edit->get_tile_layer()->tile_layer_id).hasScrolling())
		{
			if (ghost_pos.mirrorx)
			{
				RenderState off_state = state;
				off_state.transform = off_state.transform.translate({ offset.x, 0.f });
				target.draw(tile_ghost, off_state);
				target.draw(tile_text, off_state);
			}
			if (ghost_pos.mirrory)
			{
				RenderState off_state = state;
				off_state.transform = off_state.transform.translate({ 0.f, offset.y });
				target.draw(tile_ghost, off_state);
				target.draw(tile_text, off_state);
			}
			if (ghost_pos.mirrorx && ghost_pos.mirrory)
			{
				RenderState off_state = state;
				off_state.transform = off_state.transform.translate(offset);
				target.draw(tile_ghost, off_state);
				target.draw(tile_text, off_state);
			}
		}
	}

}
