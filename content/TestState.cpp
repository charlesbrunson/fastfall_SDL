
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
	stateID = ff::EngineStateID::TEST_STATE;
	clearColor = ff::Color{ 0x141013FF };

	instance = ff::CreateInstance();
	instanceID = instance->getInstanceID();
	assert(instance);

	if (ff::LevelAsset* lvlptr = ff::Resources::get<ff::LevelAsset>("map_test.tmx"))
	{
		instance->addLevel(*lvlptr);
	}
	assert(instance->getActiveLevel());
	Level* lvl = instance->getActiveLevel();

	lvl->update(0.0);
	instance->getObject().update(0.0);
	instance->getCollision().update(0.0);


	edit = std::make_unique<LevelEditor>( *lvl, false );
	edit->select_layer(-1);
	edit->select_tileset("autotile_test.tsx");
	edit->select_tile(TileID{ 0u, 0u });

	
	//std::string font_file_path = fmt::format("{}{}", FF_DATA_DIR, "data/font/pixelated.ttf");
	std::string font_file_path = fmt::format("{}{}", FF_DATA_DIR, "data/font/dogicapixel.ttf");
	font.loadFromFile(font_file_path, 8u);

	copypasta = 
R"(What the fuck did you just fucking say about me, you little bitch? I'll have you know I graduated top of my class 
in the Navy Seals, and I've been involved in numerous secret raids on Al-Quaeda, and I have over 300 confirmed kills.
I am trained in gorilla warfare and I'm the top sniper in the entire US armed forces. You are nothing to me but just 
another target. I will wipe you the fuck out with precision the likes of which has never been seen before on this 
Earth, mark my fucking words. You think you can get away with saying that shit to me over the Internet? Think again, 
fucker. As we speak I am contacting my secret network of spies across the USA and your IP is being traced right now 
so you better prepare for the storm, maggot. The storm that wipes out the pathetic little thing you call your life.
You're fucking dead, kid. I can be anywhere, anytime, and I can kill you in over seven hundred ways, and that's just 
with my bare hands. Not only am I extensively trained in unarmed combat, but I have access to the entire arsenal of 
the United States Marine Corps and I will use it to its full extent to wipe your miserable ass off the face of the 
continent, you little shit. If only you could have known what unholy retribution your little "clever" comment was 
about to bring down upon you, maybe you would have held your fucking tongue. But you couldn't, you didn't, and now 
you're paying the price, you goddamn idiot. I will shit fury all over you and you will drown in it. You're fucking 
dead, kiddo.)";

	text.set(font, "");
	text.set_color(Color::White);
	text_bg.setOutlineColor(ff::Color::Transparent);
	text_bg.setColor(ff::Color(0, 0, 0, 100));
	
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

		const TileLayer& tilelayer = edit->get_tile_layer()->tilelayer;

		Vec2f mpos = Input::getMouseWorldPosition();
		tpos = tilelayer.getTileFromWorldPos(mpos).value_or(Vec2i{});

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
					TileID tile_pos = tile.value();
					tile_pos.x = (tile_pos.x + dir.x) % tileset->getTileSize().x;
					tile_pos.y = (tile_pos.y + dir.y) % tileset->getTileSize().y;
					edit->select_tile(tile_pos);
				}
				else {
					edit->select_tile(TileID{ 0u, 0u });
				}
				LOG_INFO("tile pos = {}", edit->get_tile()->to_vec().to_string());
			});
		};
		static auto layerOnKeyPressed = [this](SDL_Scancode c, int i) {
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
				LOG_INFO("layer = {}", layer);
			});
		};

		tileOnKeyPressed(SDL_SCANCODE_LEFT,  Vec2i{ -1,  0 });
		tileOnKeyPressed(SDL_SCANCODE_RIGHT, Vec2i{  1,  0 });
		tileOnKeyPressed(SDL_SCANCODE_UP,    Vec2i{  0, -1 });
		tileOnKeyPressed(SDL_SCANCODE_DOWN,  Vec2i{  0,  1 });

		layerOnKeyPressed(SDL_SCANCODE_KP_MINUS, -1);
		layerOnKeyPressed(SDL_SCANCODE_KP_PLUS, 1);

		if (Input::getMouseInView() && (Input::isHeld(InputType::MOUSE1) || Input::isHeld(InputType::MOUSE2)))
		{
			Level* lvl = instance->getActiveLevel();

			if (Rectf{ Vec2f{}, Vec2f{lvl->size()} * TILESIZE_F }.contains(mpos)
				&& (!painting || (last_paint != tpos)))
			{
				Input::isHeld(InputType::MOUSE1)
					? edit->paint_tile(Vec2u{ tpos })
					: edit->erase_tile(Vec2u{ tpos });

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

	
	i = (i + 1) % (copypasta.size() * 2);
	text.set(font, copypasta.substr(0, std::min((size_t)i, copypasta.size())));

	text.setPosition(viewPos - Vec2f{ text.get_bounds().getSize() } / 2.f);
	text_bg.setPosition(text.getPosition());
	text_bg.setSize(text.get_bounds().getSize());
	

	if (edit) 
	{
		auto tileset = edit->get_tileset();
		auto tile = edit->get_tile();

		if (tileset && tile) 
		{

			const TileLayer& tilelayer = edit->get_tile_layer()->tilelayer;
			ghost_pos = tilelayer.getWorldPosFromTilePos(tpos);

			//LOG_INFO("{} -> {}", tpos.to_string(), worldpos.to_string());

			tile_ghost.setPosition(ghost_pos.real);

			tile_ghost.setSize({ TILESIZE_F, TILESIZE_F });
			tile_ghost.setColor(ff::Color::White().alpha(80));
			tile_ghost.setTexture(&tileset->getTexture());
			tile_ghost.setTextureRect(Rectf{
					Vec2f{ tile->to_vec() } * TILESIZE_F,
					Vec2f{ 1, 1 } * TILESIZE_F
				});
		}
	}
	else {
		tile_ghost.setColor(ff::Color::Transparent);
	}
}

void TestState::draw(ff::RenderTarget& target, ff::RenderState state) const 
{
	target.draw(instance->getScene(), state);



	if (edit && edit->get_tile_layer()) {
		target.draw(tile_ghost, state);

		Vec2f offset = -1.f * Vec2f{ edit->get_tile_layer()->tilelayer.getSize() } * TILESIZE_F;

		if (ghost_pos.mirrorx)
		{
			ff::RenderState off_state = state;
			off_state.transform = off_state.transform.translate({ offset.x, 0.f });
			target.draw(tile_ghost, off_state);
		}
		if (ghost_pos.mirrory)
		{
			ff::RenderState off_state = state;
			off_state.transform = off_state.transform.translate({ 0.f, offset.y });
			target.draw(tile_ghost, off_state);
		}
		if (ghost_pos.mirrorx && ghost_pos.mirrory)
		{
			ff::RenderState off_state = state;
			off_state.transform = off_state.transform.translate( offset );
			target.draw(tile_ghost, off_state);
		}
	}

	target.draw(text_bg, state);
	target.draw(text, state);

}
