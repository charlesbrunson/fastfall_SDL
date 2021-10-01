
#include "TestState.hpp"

#include "fastfall/game/Instance.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/render/ShapeCircle.hpp"
#include "fastfall/render/ShapeLine.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/engine/input.hpp"

#include "tilelogic/AnimLogic.hpp"


TestState::TestState()
{
	instance = ff::CreateInstance();
	instanceID = instance->getInstanceID();
	assert(instance);

	if (ff::LevelAsset* lvlptr = ff::Resources::get<ff::LevelAsset>("map_test"))
	{
		instance->addLevel(*lvlptr);
	}
	assert(instance->getActiveLevel());
	Level* lvl = instance->getActiveLevel();

	lvl->update(0.0);
	instance->getObject().update(0.0);
	instance->getCollision().update(0.0);

	stateID = ff::EngineStateID::TEST_STATE;
	clearColor = ff::Color{ 0x141013FF };

	edit = std::make_unique<LevelEditor>( *lvl, false );
	edit->select_layer(-1);
	edit->select_tileset("tile_test");
	edit->select_tile(Vec2u{ 0, 0 });

}

TestState::~TestState() {
	ff::DestroyInstance(instance->getInstanceID());
}

void TestState::update(secs deltaTime) {

	instance->getActiveLevel()->update(deltaTime);
	instance->getObject().update(deltaTime);
	instance->getTrigger().update(deltaTime);
	instance->getCollision().update(deltaTime);
	instance->getCamera().update(deltaTime);

	currKeys = SDL_GetKeyboardState(&key_count);

	if (edit) 
	{
		if (!edit->is_attached()) {
			edit->reattach();
		}
		edit->select_layer(-1);

		Vec2f mpos = Input::getMouseWorldPosition();
		tpos = Vec2u{ mpos / TILESIZE_F };

		static auto onKeyPressed = [this](SDL_Scancode c, auto&& callable) {
			if (currKeys && prevKeys && currKeys[c] && !prevKeys[c]) {
				callable();
			}
		};

		static auto tileOnKeyPressed = [this](SDL_Scancode c, Vec2i dir) {
			onKeyPressed(c, [this, dir]() {
				auto tileset = edit->get_tileset();
				auto tile = edit->get_tile();
				if (tileset && tile) {
					Vec2u tile_pos = tile.value();

					tile_pos.x = (tile_pos.x + dir.x) % tileset->getTileSize().x;
					tile_pos.y = (tile_pos.y + dir.y) % tileset->getTileSize().y;
					edit->select_tile(tile_pos);
				}
				else {
					edit->select_tile(Vec2u{ 0, 0 });
				}
				LOG_INFO("tile pos = {}", edit->get_tile()->to_string());
			});
		};

		tileOnKeyPressed(SDL_SCANCODE_LEFT,  Vec2i{ -1,  0 });
		tileOnKeyPressed(SDL_SCANCODE_RIGHT, Vec2i{  1,  0 });
		tileOnKeyPressed(SDL_SCANCODE_UP,    Vec2i{  0, -1 });
		tileOnKeyPressed(SDL_SCANCODE_DOWN,  Vec2i{  0,  1 });
		
		if (Input::getMouseInView() && (Input::isHeld(InputType::MOUSE1) || Input::isHeld(InputType::MOUSE2)))
		{
			Level* lvl = instance->getActiveLevel();

			if (Rectf{ Vec2f{}, Vec2f{lvl->size()} * TILESIZE_F }.contains(mpos)
				&& (!painting || (last_paint != tpos)))
			{

				Input::isHeld(InputType::MOUSE1)
					? edit->paint_tile(tpos)
					: edit->erase_tile(tpos);

			}

			last_paint = tpos;
			painting = true;
		}
		else {
			painting = false;
		}
	}

	if (currKeys) {
		if (!prevKeys)
			prevKeys = std::make_unique<Uint8[]>(key_count);
		std::memcpy(&prevKeys[0], currKeys, key_count);
	}
}

void TestState::predraw(secs deltaTime) {
	if (instance->want_reset)
		instance->reset();

	instance->getObject().predraw(deltaTime);
	instance->getActiveLevel()->predraw(deltaTime);

	viewPos = instance->getCamera().currentPosition;
	viewZoom = instance->getCamera().zoomFactor;

	instance->getScene().set_cam_pos(viewPos);

	tile_ghost.setColor(ff::Color::Transparent);
	if (edit) 
	{
		auto tileset = edit->get_tileset();
		auto tile = edit->get_tile();

		if (tileset && tile) 
		{
			tile_ghost.setColor(ff::Color::White().alpha(80));
			tile_ghost.setTexture(&tileset->getTexture());
			tile_ghost.setTextureRect(Rectf{
					Vec2f{ *tile } * TILESIZE_F,
					Vec2f{ 1, 1 } * TILESIZE_F
				});
			tile_ghost.setPosition(Vec2f{ tpos } * TILESIZE_F);
			tile_ghost.setSize({ TILESIZE_F, TILESIZE_F });
		}
	}
}

void TestState::draw(ff::RenderTarget& target, ff::RenderState state) const 
{
	target.draw(instance->getScene(), state);
	target.draw(tile_ghost, state);
}
